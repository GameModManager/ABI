/**
 * GameModManager ABI v1 — Stable C ABI for game modules
 *
 * This header is the single source of truth for plugin <-> engine communication.
 * Plugins include this header and call gmm_register_v1() after load.
 * Engine calls gmm_abi_version() after dlopen to verify compatibility.
 *
 * Rules:
 * - All functions use extern "C" linkage (no name mangling)
 * - Opaque handles: plugins never see engine internals
 * - String lifetime: engine owns all returned strings; plugin must copy if needed
 * - Thread safety: callbacks are called from the pipeline thread only
 * - Error handling: 0 = failure, 1 = success (expandable later)
 */

#ifndef GMM_ABI_V1_H
#define GMM_ABI_V1_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Version ── */
#define GMM_ABI_VERSION 1

/* ── Opaque handles ── */
typedef struct GmmInstance* GmmInstanceHandle;
typedef struct GmmMod* GmmModHandle;
typedef struct GmmConflictIndex* GmmConflictIndexHandle;
typedef struct GmmProfile* GmmProfileHandle;

/* ── Enums ── */
typedef enum {
    GMM_MOD_DOWNLOADED = 0,
    GMM_MOD_EXTRACTED  = 1,
    GMM_MOD_INSTALLED  = 2,
    GMM_MOD_STAGED     = 3,
    GMM_MOD_DEPLOYED   = 4,
} GmmModState;

typedef enum {
    GMM_INSTANCE_MODS      = 0,
    GMM_INSTANCE_PROFILES  = 1,
    GMM_INSTANCE_DOWNLOADS = 2,
    GMM_INSTANCE_CACHE     = 3,
    GMM_INSTANCE_LOGS      = 4,
    GMM_INSTANCE_CONFIG    = 5,
} GmmInstanceKind;

/* ── Mod file info ── */
typedef struct {
    const char* relative_path;
    uint64_t size;
} GmmModFile;

/* ── Instance API ── */
const char* gmm_instance_game_id(GmmInstanceHandle h);
const char* gmm_instance_root(GmmInstanceHandle h);
const char* gmm_instance_path_for(GmmInstanceHandle h, GmmInstanceKind kind);

/* ── Mod API ── */
const char*      gmm_mod_id(GmmModHandle h);
const char*      gmm_mod_name(GmmModHandle h);
const char*      gmm_mod_version(GmmModHandle h);
GmmModState      gmm_mod_state(GmmModHandle h);
void             gmm_mod_set_state(GmmModHandle h, GmmModState state);
GmmModFile       gmm_mod_file_at(GmmModHandle h, size_t index);
size_t           gmm_mod_file_count(GmmModHandle h);

/* ── ConflictIndex API ── */
void gmm_conflict_add_file(GmmConflictIndexHandle h,
                           const char* relative_path,
                           const char* mod_id,
                           uint32_t priority);
void gmm_conflict_remove_mod(GmmConflictIndexHandle h, const char* mod_id);
const char* gmm_conflict_winner(GmmConflictIndexHandle h, const char* relative_path);

/* ── Profile API ── */
void     gmm_profile_add_mod(GmmProfileHandle h, const char* mod_id, int enabled);
void     gmm_profile_remove_mod(GmmProfileHandle h, const char* mod_id);
void     gmm_profile_set_enabled(GmmProfileHandle h, const char* mod_id, int enabled);
void     gmm_profile_move_mod(GmmProfileHandle h, const char* mod_id, uint32_t new_position);
uint32_t gmm_profile_priority_of(GmmProfileHandle h, const char* mod_id);

/* ── Pipeline stage callback ── */
typedef int (*GmmStageFn)(GmmModHandle mod, GmmInstanceHandle instance,
                          GmmConflictIndexHandle conflicts, GmmProfileHandle profile,
                          void* user_data);

/* ── Order encoding callback ── */
typedef int (*GmmOrderEncodingFn)(const char* const* ordered_mod_ids,
                                  size_t count,
                                  const char* output_path,
                                  void* user_data);

/* ── Deployment strategy callbacks ── */
typedef int (*GmmDeployFn)(const char* source, const char* target, void* user_data);
typedef int (*GmmRemoveFn)(const char* target, void* user_data);

/* ── Registration context ── */
typedef struct GmmRegistrationCtx GmmRegistrationCtx;

struct GmmRegistrationCtx {
    /* Identity — pure data, no behavior */
    void (*register_identity)(GmmRegistrationCtx* ctx,
                              uint32_t steam_appid,
                              const char* gog_id,
                              const char* epic_namespace,
                              const char* nexus_domain,
                              const char* exe_windows,
                              const char* exe_linux,
                              const char* exe_macos);

    /* Stage claim — exclusive ownership of a pipeline stage */
    void (*register_stage_claim)(GmmRegistrationCtx* ctx,
                                 const char* stage_name,
                                 GmmStageFn fn,
                                 int priority);

    /* Order encoding hook — plugins.txt / metadata.xml writer */
    void (*register_order_encoding)(GmmRegistrationCtx* ctx,
                                    GmmOrderEncodingFn fn);

    /* Deploy strategy — symlink / hardlink / vfs */
    void (*register_deploy_strategy)(GmmRegistrationCtx* ctx,
                                     GmmDeployFn deploy_fn,
                                     GmmRemoveFn remove_fn);

    /* Tool registration — LOOT, BodySlide, etc. */
    void (*register_tool)(GmmRegistrationCtx* ctx,
                          const char* tool_id,
                          const char* kind,
                          void (*invoke_fn)(void* user_data),
                          void* user_data);

    /* Capability registration — tells UI which tabs to show for this game.
     * capability: "plugins", "archives", "saves", "downloads"
     * display_name: tab label (e.g. "Plugins")
     * data_path: relative path where these are stored (e.g. "Data/", "Documents/My Games/...")
     * description: human-readable hint
     * For downloads: protocol_handler ("nxm", "workshop"), website_domain, supported_platforms (comma-separated)
     */
    void (*register_capability)(GmmRegistrationCtx* ctx,
                                const char* capability,
                                const char* display_name,
                                const char* data_path,
                                const char* description,
                                const char* protocol_handler,
                                const char* website_domain,
                                const char* supported_platforms);

    /* User data passed back to all callbacks */
    void* user_data;
};

/* ── Plugin entry point ── */
/* Called once after load. Plugin calls ctx->register_* as needed. */
extern void gmm_register_v1(GmmRegistrationCtx* ctx);

/* ── Version guard ── */
/* Host checks this after dlopen to verify ABI compatibility */
extern uint32_t gmm_abi_version(void);

#ifdef __cplusplus
}
#endif

#endif /* GMM_ABI_V1_H */
