/**
 * GameModManager ABI v2 — Clean-break C ABI for game modules (v2.0 foundation)
 *
 * v2 is a redesign of the v1 ABI with MO2-parity interface terminology. It
 * ships side-by-side with v1; v2 does NOT need backwards compatibility with v1.
 *
 * Rules (same as v1):
 * - All functions use extern "C" linkage (no name mangling)
 * - Opaque handles: plugins never see engine internals
 * - String lifetime: engine owns all returned strings; plugin must copy if needed
 * - Thread safety: callbacks are called from the pipeline thread only
 * - Error handling: 0 = failure, 1 = success
 *
 * Differences from v1:
 * - Interface-grouped registration structs (not a monolithic ctx)
 * - New entry point gmm_register_v2()
 * - Richer, typed registration (game info, previews, diagnostics, requirements,
 *   file mappers, mod pages, save parsers) with explicit game_id scoping.
 */

#ifndef GMM_ABI_V2_H
#define GMM_ABI_V2_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Version --
 * Guarded so a translation unit that includes BOTH gmm_abi_v1.h and
 * gmm_abi_v2.h (the engine loader) does not get a redefinition warning. A v2
 * plugin that includes only this header still sees GMM_ABI_VERSION == 2. */
#ifndef GMM_ABI_VERSION
#define GMM_ABI_VERSION 2
#endif

/* -- Plugin identity info -- */
typedef struct {
    const char* name;
    const char* author;
    const char* version;
    const char* description;
} GmmPluginInfo;

/* -- Game identity info -- */
typedef struct {
    const char* game_id;
    const char* display_name;
    uint32_t steam_appid;
    const char* gog_id;
    const char* epic_namespace;
    const char* nexus_domain;
    const char* exe_windows;
    const char* exe_linux;
    const char* exe_macos;
} GmmGameInfo;

/* -- Preview info -- */
typedef struct {
    const char* file_extension;  // e.g. ".anm2", ".dds", ".nif"
    void* preview_data;          // opaque, passed back to preview_fn
} GmmPreviewInfo;

/* -- Diagnostic problem -- */
typedef struct {
    const char* short_description;
    const char* full_description;
    int has_guided_fix;
    void (*start_guided_fix)(void* user_data);
    void* user_data;
} GmmDiagnosticProblem;

/* -- File mapping entry -- */
typedef struct {
    const char* source;  // virtual path
    const char* target;  // real path
} GmmFileMapping;

/* -- Plugin requirement -- */
typedef struct {
    const char* type;     // "plugin", "game", "diagnose"
    const char* name;     // required plugin/game name
    const char* message;  // error message if not met
} GmmPluginRequirement;

/* -- Save game data (reused from v1) -- */
typedef struct {
    char* file_path;
    char* game_id;
    int64_t creation_time;
    char* pc_name;
    int32_t pc_level;
    char* pc_location;
    uint32_t save_number;
    uint32_t plugin_count;
    char* plugins[256];
    uint32_t light_plugin_count;
    char* light_plugins[256];
} GmmSaveDataV2;

/* -- Callback types -- */
/* Preview generator: returns QWidget* (opaque void*) */
typedef void* (*GmmPreviewFn)(const char* file_path, void* preview_data, void* user_data);

/* Diagnostics: returns array of problems */
typedef GmmDiagnosticProblem* (*GmmDiagnoseFn)(size_t* out_count, void* user_data);

/* File mapper: returns array of mappings */
typedef GmmFileMapping* (*GmmFileMapperFn)(size_t* out_count, void* user_data);

/* Requirements: returns array of requirements */
typedef GmmPluginRequirement* (*GmmRequirementsFn)(size_t* out_count, void* user_data);

/* Settings getter/setter */
typedef const char* (*GmmSettingsGetFn)(const char* key, void* user_data);
typedef void (*GmmSettingsSetFn)(const char* key, const char* value, void* user_data);

/* Hook callback */
typedef void (*GmmHookFnV2)(const char* tag, void* data, void* user_data);

/* Stage callback */
typedef int (*GmmStageFnV2)(void* mod, void* instance, void* conflicts, void* profile, void* user_data);

/* Order encoding */
typedef int (*GmmOrderEncodingFnV2)(const char* const* ordered_mod_ids, size_t count, const char* output_path, void* user_data);

/* Deploy strategy */
typedef int (*GmmDeployFnV2)(const char* source, const char* target, void* user_data);
typedef int (*GmmRemoveFnV2)(const char* target, void* user_data);

/* Save parser */
typedef int (*GmmSaveParserFnV2)(const char* path, const char* game_id, GmmSaveDataV2* out, void* user_data);

/* Tool invoke */
typedef void (*GmmToolInvokeFn)(void* user_data);

/* ModPage download handler */
typedef int (*GmmModPageDownloadFn)(const char* url, const char* output_path, void* user_data);

/* Sort function */
typedef const char* const* (*GmmSortFn)(const char* const* mod_folders, size_t count, void* user_data);

/* -- Registration context (the main struct) -- */
typedef struct GmmRegistrationCtxV2 {
    void* user_data;  /* passed back to all callbacks */

    /* IPlugin interface group */
    void (*register_plugin)(GmmRegistrationCtxV2* ctx, GmmPluginInfo info);
    void (*register_settings)(GmmRegistrationCtxV2* ctx, const char* const* keys, const char* const* values, size_t count);
    void (*register_settings_tab)(GmmRegistrationCtxV2* ctx, const char* title, const char* const* keys, const char* const* types, const char* const* defaults, const char* const* options, size_t count);
    void (*register_requirements)(GmmRegistrationCtxV2* ctx, GmmRequirementsFn fn, void* user_data);
    void (*register_diagnostics)(GmmRegistrationCtxV2* ctx, const char* game_id, GmmDiagnoseFn fn, void* user_data);

    /* IPluginGame interface group */
    void (*register_game)(GmmRegistrationCtxV2* ctx, GmmGameInfo info);
    void (*register_game_feature)(GmmRegistrationCtxV2* ctx, const char* feature_type, int priority, const char* const* folder_names, size_t folder_count, const char* const* file_extensions, size_t extension_count);
    void (*register_game_feature_data)(GmmRegistrationCtxV2* ctx, const char* feature_type, int priority, const char* const* keys, const char* const* values, size_t count);
    void (*register_hook)(GmmRegistrationCtxV2* ctx, const char* tag, const char* data, GmmHookFnV2 fn, int priority, void* user_data);
    void (*register_order_encoding)(GmmRegistrationCtxV2* ctx, GmmOrderEncodingFnV2 fn, void* user_data);
    void (*register_deploy_strategy)(GmmRegistrationCtxV2* ctx, GmmDeployFnV2 deploy_fn, GmmRemoveFnV2 remove_fn, void* user_data);
    void (*register_file_mapper)(GmmRegistrationCtxV2* ctx, const char* game_id, GmmFileMapperFn fn, void* user_data);
    void (*register_sort_provider)(GmmRegistrationCtxV2* ctx, GmmSortFn sort_fn, void* user_data);

    /* IPluginInstaller interface group */
    void (*register_stage_claim)(GmmRegistrationCtxV2* ctx, const char* stage_name, GmmStageFnV2 fn, int priority);
    void (*register_wildcard_stage_claim)(GmmRegistrationCtxV2* ctx, const char* game_id, const char* stage_name, GmmStageFnV2 fn, int priority);

    /* IPluginPreview interface group */
    void (*register_preview)(GmmRegistrationCtxV2* ctx, GmmPreviewInfo info, GmmPreviewFn fn, void* user_data);

    /* IPluginTool interface group */
    void (*register_tool)(GmmRegistrationCtxV2* ctx, const char* tool_id, const char* kind, GmmToolInvokeFn fn, void* user_data);

    /* IPluginModPage interface group */
    void (*register_modpage)(GmmRegistrationCtxV2* ctx, const char* url, GmmModPageDownloadFn fn, void* user_data);

    /* IPluginSaveParser interface group */
    void (*register_save_parser)(GmmRegistrationCtxV2* ctx, const char* game_id, GmmSaveParserFnV2 fn, int priority, void* user_data);
} GmmRegistrationCtxV2;

/* -- Plugin entry point -- */
/* Called once after load. Plugin calls ctx->register_* as needed. */
extern void gmm_register_v2(GmmRegistrationCtxV2* ctx);

/* -- Version guard -- */
/* Host checks this after dlopen to verify ABI compatibility. A v2 plugin
 * returns 2. */
extern uint32_t gmm_abi_version(void);

#ifdef __cplusplus
}
#endif

#endif /* GMM_ABI_V2_H */
