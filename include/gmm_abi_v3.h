/**
 * GameModManager ABI v3 - Self-describing plugin descriptor (entity/component)
 *
 * v3 ships side-by-side with v1 and v2. A v3 plugin is an entity table
 * (games) plus a component table (capabilities) handed to the engine as one
 * self-describing blob through ONE stable entry symbol. The engine reads
 * gmm_abi_version() first, dispatches on an immutable adapter table
 * (v1/v2/v3), and commits or refuses the blob atomically. Plugins of
 * DIFFERENT API versions load in parallel side by side and share the same
 * registries.
 *
 * A plugin exports exactly two symbols, forever. There is no
 * gmm_register_v3 and there never will be a version-suffixed v3+ symbol:
 * a future API version bumps the number returned by gmm_abi_version() and
 * appends fields at the struct tails. The symbol set is frozen at two.
 *
 *     extern "C" uint32_t             gmm_abi_version(void);  // returns 3
 *     extern "C" const GmmPluginDesc* gmm_entry(const GmmHostServices* host);
 *
 * Rules:
 * - All functions use extern "C" linkage (no name mangling)
 * - No C++ types cross the boundary: payloads are POD structs plus one
 *   generic trampoline (GmmInvokeFn) per capability object
 * - NO struct_size anywhere (v1/v2 style): gmm_abi_version() is the layout
 *   gate. The host only ever reads a blob whose declared version it
 *   supports, so it knows the exact layout. Evolution = append fields at
 *   the struct tail and bump GMM_API_VERSION
 * - String lifetime: desc/entity/capability strings are plugin-owned
 *   static data, valid until the plugin unloads. Strings RETURNED by host
 *   services (resolve_path) are Core-owned for the duration of the call on
 *   the calling thread; the plugin must copy if it needs them later
 * - Payload ownership is unchanged from v2: the plugin mallocs every
 *   string/buffer it writes into a payload struct, Core frees
 * - Exceptions never cross: the plugin-side thunk (SDK) catches and
 *   converts to GMM_ERR; the host reads the args error buffer only on
 *   GMM_ERR
 * - Thread safety: gmm_entry is called once after load, from the load
 *   thread; invoke callbacks are called from the pipeline thread only
 * - This header includes ONLY <stdint.h>/<stddef.h> and defines NO payload
 *   structs. Payload types (GmmSaveDataV2 and friends) stay in
 *   gmm_abi_v2.h; a v3 TU that parses a v2 payload includes that header
 *   itself. GMM_ABI_VERSION (frozen headers) and GMM_API_VERSION (this
 *   header) intentionally coexist in such a TU.
 */

#ifndef GMM_ABI_V3_H
#define GMM_ABI_V3_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Version --
 * The ONE version declaration for v3+. Deliberately NOT named
 * GMM_ABI_VERSION: that name belongs to the frozen v1/v2 headers, and a v3
 * TU may legitimately include gmm_abi_v2.h for payload structs, so both
 * macros coexist by design. */
#define GMM_API_VERSION 3u

/* -- Symbol visibility --
 * Plugins are built with -fvisibility=hidden; the two exports carry
 * GMM_EXPORT so they stay visible. */
#if defined(__GNUC__) || defined(__clang__)
#define GMM_EXPORT __attribute__((visibility("default")))
#elif defined(_WIN32)
#define GMM_EXPORT __declspec(dllexport)
#else
#define GMM_EXPORT
#endif

/* -- Capability ids --
 * String ids exist only at this C-ABI border (cross-.so dispatch) and in
 * the TOML descriptor; the typed C++ SDK resolves them from the class.
 * The capability manifest (engine side) is the drift-checked authority;
 * new ids append, existing ids never change. */
#define GMM_CAPID_SAVES_PARSE "saves.parse"
#define GMM_CAPID_SAVES_OVERLAY "saves.overlay"
#define GMM_CAPID_PREVIEW_FILE "preview.file"
#define GMM_CAPID_ANIMATION_PARSE "animation.parse"
#define GMM_CAPID_SORT_PROVIDER "sort.provider"
#define GMM_CAPID_TOOL "tool"
#define GMM_CAPID_INSTALLER_STAGE "installer.stage"
#define GMM_CAPID_DETECTION_VALIDATOR "detection.validator"
#define GMM_CAPID_EVENT_SUBSCRIBE "event.subscribe"
#define GMM_CAPID_UI_DIAGNOSTICS "ui.diagnostics"
#define GMM_CAPID_UI_SETTINGS "ui.settings"
#define GMM_CAPID_LOAD_ORDER "load_order.encoding"
#define GMM_CAPID_INSTALL_CHECKER "install.checker"
#define GMM_CAPID_PLUGIN_SETTINGS "plugin.settings"

/* -- Method ids --
 * Append-only, never renumbered, grouped per capability. The invoke
 * trampoline switches on these. */
enum { GMM_SAVES_PARSE_FULL = 1, GMM_SAVES_PARSE_FAST = 2 };
enum { GMM_OVERLAY_BUILD = 1 };
enum { GMM_PREVIEW_CREATE = 1 };
enum { GMM_ANIM_PARSE = 1 };
enum { GMM_SORT_RUN = 1 };
enum { GMM_TOOL_INVOKE = 1 };
enum { GMM_STAGE_INSTALL = 1 };
enum { GMM_VALIDATE_DIR = 1 };
enum { GMM_EVENT_DELIVER = 1 };
enum { GMM_DIAGNOSE_COLLECT = 1 };
enum { GMM_ORDER_WRITE = 1 };

/* -- Invoke result codes -- */
#define GMM_OK 0           /* success */
#define GMM_ERR -1         /* failure; args error buffer is readable */
#define GMM_UNSUPPORTED -2 /* method id or optional path not supported */

/* -- Host log levels (values match the engine LogLevel order) -- */
enum { GMM_LOG_DEBUG = 0, GMM_LOG_INFO = 1, GMM_LOG_WARN = 2, GMM_LOG_ERROR = 3 };

/* -- Host services --
 * The engine hands this to gmm_entry. log and resolve_path are the only
 * services (expand_path is deferred until a plugin needs it). */
typedef struct GmmHostServices {
  uint32_t version; /* host API level (GMM_API_VERSION the host built with) */
  void *host_ctx;   /* Core-owned; the plugin never dereferences it */
  void (*log)(void *host_ctx, int level, const char *msg);
  /* Same contract as the v2 resolve_file host call: Core owns the
   * returned string for the duration of the call on this thread. */
  const char *(*resolve_path)(void *host_ctx, const char *root, const char *rel);
} GmmHostServices;

/* -- The one trampoline --
 * One function pointer per capability object; the plugin-side thunk
 * (compiled into the plugin) switches on the method id and calls the
 * author's class. args/out are method-specific PODs documented per
 * capability in the SDK headers. */
typedef int32_t (*GmmInvokeFn)(void *self, uint32_t method, void *args, void *out);

/* -- Capability object (one component) -- */
typedef struct GmmCapabilityObject {
  const char *id; /* GMM_CAPID_* */
  /* Game entity this capability binds to. NULL/"" = app-wide (not
   * game-scoped; plugin.settings is the current app-wide id). A plugin
   * supporting N games declares N entries sharing self/invoke. */
  const char *game_id;
  uint32_t version;   /* method-set version, starts at 1; a cap version
                           newer than the host knows is dropped with a
                           warning, only that binding */
  int32_t priority;   /* higher wins; equal priority from different
                           plugins = first by sorted load order wins */
  void *self;         /* plugin-owned, alive until unload */
  GmmInvokeFn invoke; /* NULL = data-only capability */
} GmmCapabilityObject;

/* -- Game entity (one game record) --
 * Identity is declared by exactly ONE provider per game_id; a duplicate
 * game_id inside one desc refuses the whole desc. */
typedef struct GmmGameEntity {
  uint32_t version;         /* entity struct version, starts at 1 */
  const char *game_id;      /* [REQ] canonical id; aliases are for legacy
                                     instance migration only */
  const char *display_name; /* [REQ] human-readable name */
  /* [OPT] store ids; absent = no store feed declared for this game */
  const uint32_t *steam_appids;
  uint32_t steam_appid_count;
  const char *gog_id;
  const char *epic_namespace;
  const char *nexus_domain; /* [OPT] absent = no Nexus wiring */
  /* [OPT] legacy instance ids canonicalized once at instance load */
  const char *const *aliases;
  uint32_t alias_count;
  const char *mods_subpath; /* [REQ] mods dir relative to the game dir */
  /* [OPT] game executables; absent = no launch wiring */
  const char *const *executables;
  uint32_t executable_count;
} GmmGameEntity;

/* -- Plugin descriptor blob --
 * gmm_entry returns a pointer to ONE static instance of this (plugin-owned,
 * valid until unload). The host reads it after gmm_entry returns and never
 * writes it. */
typedef struct GmmPluginDesc {
  uint32_t version;           /* GMM_API_VERSION the plugin was built for;
                                       must equal gmm_abi_version() */
  const char *name;           /* [REQ] plugin name */
  const char *author;         /* [OPT] NULL/"" = uncredited */
  const char *version_string; /* [OPT] plugin version string */
  const char *description;    /* [OPT] one-line summary */
  const GmmGameEntity *games;
  uint32_t game_count;
  const GmmCapabilityObject *caps;
  uint32_t cap_count;
} GmmPluginDesc;

/* -- The two exports, forever -- */
GMM_EXPORT extern uint32_t gmm_abi_version(void);
GMM_EXPORT extern const GmmPluginDesc *gmm_entry(const GmmHostServices *host);

#ifdef __cplusplus
}
#endif

#endif /* GMM_ABI_V3_H */
