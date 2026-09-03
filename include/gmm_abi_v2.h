/**
 * GameModManager ABI v2 — Clean-break C ABI for game modules
 *
 * v2 is a redesign of v1 with MO2-parity interface terminology. Ships
 * side-by-side with v1; no backwards compat needed.
 *
 * Rules:
 * - All functions use extern "C" linkage (no name mangling)
 * - Opaque handles: plugins never see engine internals
 * - String lifetime: engine owns all returned strings; plugin must copy
 * - Thread safety: callbacks are called from the pipeline thread only
 * - Error handling: 0 = failure, 1 = success
 */

#ifndef GMM_ABI_V2_H
#define GMM_ABI_V2_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -- Version -- */
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
    const char* file_extension;
    void* preview_data;
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
    const char* source;
    const char* target;
} GmmFileMapping;

/* -- Plugin requirement -- */
typedef struct {
    const char* type;
    const char* name;
    const char* message;
} GmmPluginRequirement;

/* -- Tab info for batched registration -- */
typedef struct {
    const char* capability;
    const char* display_name;
    const char* data_path;
    const char* description;
    const char* protocol_handler;
    const char* website_domain;
    const char* supported_platforms;
    const char* insert_before;
    const char* insert_after;
} GmmTabInfo;

/* -- Save game data -- */
#define GMM_SAVE_MAX_PLUGINS 256

typedef struct {
    char* file_path;
    char* game_id;
    int64_t creation_time;
    char* pc_name;
    int32_t pc_level;
    char* pc_location;
    uint32_t save_number;
    uint32_t plugin_count;
    char* plugins[GMM_SAVE_MAX_PLUGINS];
    uint32_t light_plugin_count;
    char* light_plugins[GMM_SAVE_MAX_PLUGINS];
} GmmSaveDataV2;

/* -- Animation parser types -- */
typedef struct GmmAnimationLayerV2 {
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    uint8_t* rgba_pixels;
    size_t pixel_count;
} GmmAnimationLayerV2;

typedef struct GmmAnimationFrameV2 {
    float delay_ms;
    GmmAnimationLayerV2* layers;
    size_t layer_count;
} GmmAnimationFrameV2;

/* On-demand render callback: renders a single frame at the given time.
 * raw_animation is the plugin-owned opaque pointer from GmmAnimationDataV2.
 * time_ms is the time in milliseconds from the animation start.
 * out_width/out_height receive the dimensions of the returned pixel buffer.
 * Returns malloc'd RGBA pixels (caller frees), or NULL on failure. */
typedef uint8_t* (*GmmAnimationRenderFn)(void* raw_animation, float time_ms,
                                         int32_t* out_width, int32_t* out_height);

typedef struct GmmAnimationStateV2 {
    char* name;
    int32_t canvas_width;
    int32_t canvas_height;
    GmmAnimationFrameV2* frames;
    size_t frame_count;
    void* raw_animation; /* opaque pointer to plugin-owned raw keyframe data */
    GmmAnimationRenderFn render_frame; /* on-demand render callback, or NULL */
} GmmAnimationStateV2;

typedef struct GmmAnimationDataV2 {
    float fps;
    int32_t canvas_width;
    int32_t canvas_height;
    GmmAnimationFrameV2* frames;
    size_t frame_count;
    GmmAnimationStateV2* states;
    size_t state_count;
    void* raw_animation; /* opaque pointer to plugin-owned raw keyframe data */
    GmmAnimationRenderFn render_frame; /* on-demand render callback, or NULL */
} GmmAnimationDataV2;

/* Returns non-zero on success. Caller owns all heap allocations in out. */
typedef int (*GmmAnimationParserFnV2)(const char* file_path,
                                      const char* base_dir,
                                      GmmAnimationDataV2* out,
                                      void* user_data);

/* -- Callback types -- */

typedef void* (*GmmPreviewFn)(const char* file_path, void* preview_data, void* user_data);
typedef GmmDiagnosticProblem* (*GmmDiagnoseFn)(size_t* out_count, void* user_data);
typedef GmmFileMapping* (*GmmFileMapperFn)(size_t* out_count, void* user_data);
typedef GmmPluginRequirement* (*GmmRequirementsFn)(size_t* out_count, void* user_data);
typedef void (*GmmHookFnV2)(const char* tag, void* data, void* user_data);
typedef int (*GmmStageFnV2)(void* mod, void* instance, void* conflicts, void* profile, void* user_data);
typedef int (*GmmOrderEncodingFnV2)(const char* const* ordered_mod_ids, size_t count, const char* output_path, void* user_data);
typedef int (*GmmDeployFnV2)(const char* source, const char* target, void* user_data);
typedef int (*GmmRemoveFnV2)(const char* target, void* user_data);
typedef int (*GmmSaveParserFnV2)(const char* path, const char* game_id, GmmSaveDataV2* out, void* user_data);
typedef void (*GmmToolInvokeFn)(void* user_data);
typedef int (*GmmModPageDownloadFn)(const char* url, const char* output_path, void* user_data);
typedef const char* const* (*GmmSortFn)(const char* const* mod_folders, size_t count, void* user_data);
typedef char* (*GmmResolveFileFn)(const char* root, const char* relative_path, void* user_data);

/* -- Registration context (the main struct) -- */
typedef struct GmmRegistrationCtxV2 {
    void* user_data;

    /* IPlugin */
    void (*register_plugin)(GmmRegistrationCtxV2* ctx, GmmPluginInfo info);
    void (*register_settings)(GmmRegistrationCtxV2* ctx, const char* const* keys, const char* const* values, size_t count);
    void (*register_settings_tab)(GmmRegistrationCtxV2* ctx, const char* title, const char* const* keys, const char* const* types, const char* const* defaults, const char* const* options, size_t count);
    void (*register_requirements)(GmmRegistrationCtxV2* ctx, GmmRequirementsFn fn, void* user_data);
    void (*register_diagnostics)(GmmRegistrationCtxV2* ctx, const char* game_id, GmmDiagnoseFn fn, void* user_data);
    void (*register_category)(GmmRegistrationCtxV2* ctx, const char* category);
    void (*register_categories)(GmmRegistrationCtxV2* ctx, const int* ids, const char* const* names, const int* parent_ids, size_t count);

    /* IPluginGame */
    void (*register_game)(GmmRegistrationCtxV2* ctx, GmmGameInfo info);
    void (*register_game_feature)(GmmRegistrationCtxV2* ctx, const char* feature_type, int priority, const char* const* folder_names, size_t folder_count, const char* const* file_extensions, size_t extension_count);
    void (*register_game_feature_data)(GmmRegistrationCtxV2* ctx, const char* feature_type, int priority, const char* const* keys, const char* const* values, size_t count);
    void (*register_hook)(GmmRegistrationCtxV2* ctx, const char* tag, const char* data, GmmHookFnV2 fn, int priority, void* user_data);
    void (*register_order_encoding)(GmmRegistrationCtxV2* ctx, GmmOrderEncodingFnV2 fn, void* user_data);
    void (*register_deploy_strategy)(GmmRegistrationCtxV2* ctx, GmmDeployFnV2 deploy_fn, GmmRemoveFnV2 remove_fn, void* user_data);
    void (*register_file_mapper)(GmmRegistrationCtxV2* ctx, const char* game_id, GmmFileMapperFn fn, void* user_data);
    void (*register_sort_provider)(GmmRegistrationCtxV2* ctx, GmmSortFn sort_fn, void* user_data);
    void (*register_tab)(GmmRegistrationCtxV2* ctx, const char* capability, const char* display_name, const char* data_path, const char* description, const char* protocol_handler, const char* website_domain, const char* supported_platforms, const char* insert_before, const char* insert_after);

    /* IPluginInstaller */
    void (*register_stage_claim)(GmmRegistrationCtxV2* ctx, const char* stage_name, GmmStageFnV2 fn, int priority);
    void (*register_wildcard_stage_claim)(GmmRegistrationCtxV2* ctx, const char* game_id, const char* stage_name, GmmStageFnV2 fn, int priority);
    struct GmmHostUi {
        int (*fomod_wizard)(void* mod, char* out_json, size_t out_capacity);
    } host_ui;

    /* IPluginPreview */
    void (*register_preview)(GmmRegistrationCtxV2* ctx, const char* file_extension, void* preview_data, GmmPreviewFn fn, void* user_data);

    /* IPluginTool */
    void (*register_tool)(GmmRegistrationCtxV2* ctx, const char* tool_id, const char* kind, GmmToolInvokeFn fn, void* user_data);

    /* IPluginModPage */
    void (*register_modpage)(GmmRegistrationCtxV2* ctx, const char* url, GmmModPageDownloadFn fn, void* user_data);

    /* IPluginSaveParser */
    void (*register_save_parser)(GmmRegistrationCtxV2* ctx, const char* game_id, GmmSaveParserFnV2 fn, int priority, void* user_data);

    /* IPluginAnimationParser -- registers an animation file parser.
     * NULL game_id = non-game-specific (applies to every game). */
    void (*register_animation_parser)(GmmRegistrationCtxV2* ctx, const char* game_id, const char* file_extension, GmmAnimationParserFnV2 fn, int priority, void* user_data);

    /* Host services -- resolve a case-sensitive relative path against a root */
    GmmResolveFileFn resolve_file;
    void* resolve_file_user_data;

    /* -- Additive slots (v2.1+) -- old plugins ignore these -- */

    /* Batched tab registration -- registers multiple tabs in one call.
     * Preferred over repeated register_tab() for readability. */
    void (*register_tabs)(GmmRegistrationCtxV2* ctx,
                          const GmmTabInfo* tabs, size_t count);

    /* Event subscription -- receive notifications for named events.
     * The engine calls fn(event, data, user_data) when the event fires. */
    void (*subscribe)(GmmRegistrationCtxV2* ctx, const char* event,
                      void (*fn)(const char* event, void* data,
                                 void* user_data),
                      void* user_data);
} GmmRegistrationCtxV2;

/* -- Plugin entry point -- */
extern void gmm_register_v2(GmmRegistrationCtxV2* ctx);

/* -- Version guard -- */
extern uint32_t gmm_abi_version(void);

#ifdef __cplusplus
}
#endif

#endif /* GMM_ABI_V2_H */
