/**
 * plugin.hpp - Header-only C++ wrapper for the GMM ABI v2
 *
 * Hides the raw C function-pointer verbosity behind a fluent, chainable API:
 *
 *   extern "C" void gmm_register(GmmRegistrationCtxV2* raw) {
 *     gmm::Ctx ctx{raw};
 *     ctx.game({...})
 *        .feature<MyModDataChecker>()
 *        .tabs({tab1, tab2})
 *        .categories({cat1, cat2})
 *        .hook("tag", myFn, 100)
 *        .subscribe("event", myEventFn);
 *   }
 *
 * Or use the base-class DX:
 *
 *   class MyPlugin : public gmm::Plugin { ... };
 *   REGISTER_PLUGIN(MyPlugin)
 *
 * ABI stability:
 *   - This header adds ZERO new C ABI slots beyond what gmm_abi_v2.h declares.
 *   - It is purely a compile-time convenience; the resulting .so exports the
 *     same symbols as a hand-written gmm_register + gmm_abi_version pair.
 */

#ifndef GMM_PLUGIN_HPP
#define GMM_PLUGIN_HPP

#include "gmm_abi_v2.h"

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <vector>

namespace gmm {

// -- Tab descriptor (mirrors GmmTabInfo but with C++ naming) --
struct Tab {
  const char* capability;
  const char* display_name;
  const char* data_path;
  const char* description;
  const char* protocol_handler;
  const char* website_domain;
  const char* supported_platforms;
  const char* insert_before;
  const char* insert_after;

  // Convenience: implicit conversion to the C ABI struct
  operator GmmTabInfo() const {
    return {capability, display_name, data_path, description,
            protocol_handler,      website_domain, supported_platforms,
            insert_before,         insert_after};
  }
};

// -- Category descriptor --
struct Category {
  int id;
  const char* name;
  int parent_id;
};

// -- Callback types for hooks/events --
using HookFn = GmmHookFnV2;
using EventFn = void (*)(const char* event, void* data, void* user_data);

// -- Feature concept: any type with a register() method --
// The engine calls register_feature(ctx, shared_ptr<IFeature>).
// For now we keep it simple: wrap each registration call directly.
// A typed feature system can be layered on later.

// -- Ctx: fluent wrapper around GmmRegistrationCtxV2 --
struct Ctx {
  GmmRegistrationCtxV2* raw;

  explicit Ctx(GmmRegistrationCtxV2* r) : raw(r) {}

  // -- Game identity --
  Ctx& game(const GmmGameInfo& g) {
    raw->register_game(raw, g);
    return *this;
  }

  // -- Plugin metadata --
  Ctx& plugin(const GmmPluginInfo& info) {
    raw->register_plugin(raw, info);
    return *this;
  }

  // -- Single category --
  Ctx& category(const char* name) {
    raw->register_category(raw, name);
    return *this;
  }

  // -- Batched categories --
  Ctx& categories(std::initializer_list<Category> cats) {
    // Build parallel arrays for the C ABI call
    std::vector<int> ids;
    std::vector<const char*> names;
    std::vector<int> parent_ids;
    ids.reserve(cats.size());
    names.reserve(cats.size());
    parent_ids.reserve(cats.size());
    for (auto& c : cats) {
      ids.push_back(c.id);
      names.push_back(c.name);
      parent_ids.push_back(c.parent_id);
    }
    raw->register_categories(raw, ids.data(), names.data(),
                             parent_ids.data(), cats.size());
    return *this;
  }

  // -- Single tab --
  Ctx& tab(const Tab& t) {
    raw->register_tab(raw, t.capability, t.display_name, t.data_path,
                      t.description, t.protocol_handler, t.website_domain,
                      t.supported_platforms, t.insert_before, t.insert_after);
    return *this;
  }

  // -- Batched tabs (uses the new ABI slot) --
  Ctx& tabs(std::initializer_list<Tab> tab_list) {
    // If the engine supports batched registration, use it
    if (raw->register_tabs) {
      // Convert to GmmTabInfo array
      std::vector<GmmTabInfo> c_tabs;
      c_tabs.reserve(tab_list.size());
      for (auto& t : tab_list) {
        c_tabs.push_back(t);
      }
      raw->register_tabs(raw, c_tabs.data(), c_tabs.size());
    } else {
      // Fallback: loop over register_tab (old engine)
      for (auto& t : tab_list) {
        raw->register_tab(raw, t.capability, t.display_name, t.data_path,
                          t.description, t.protocol_handler, t.website_domain,
                          t.supported_platforms, t.insert_before,
                          t.insert_after);
      }
    }
    return *this;
  }

  // -- Hook (data-only, no callback) --
  Ctx& hook(const char* tag, const char* data, int prio = 0) {
    raw->register_hook(raw, tag, data, nullptr, prio, nullptr);
    return *this;
  }

  // -- Hook with callback --
  Ctx& hook(const char* tag, HookFn fn, int prio, void* user_data = nullptr,
            const char* data = nullptr) {
    raw->register_hook(raw, tag, data, fn, prio, user_data);
    return *this;
  }

  // -- Event subscription --
  Ctx& subscribe(const char* event, EventFn fn,
                 void* user_data = nullptr) {
    if (raw->subscribe) {
      raw->subscribe(raw, event, fn, user_data);
    }
    return *this;
  }

  // -- Order encoding --
  Ctx& order_encoding(GmmOrderEncodingFnV2 fn,
                      void* user_data = nullptr) {
    raw->register_order_encoding(raw, fn, user_data);
    return *this;
  }

  // -- Deploy strategy --
  Ctx& deploy_strategy(GmmDeployFnV2 deploy_fn, GmmRemoveFnV2 remove_fn,
                       void* user_data = nullptr) {
    raw->register_deploy_strategy(raw, deploy_fn, remove_fn, user_data);
    return *this;
  }

  // -- File mapper --
  Ctx& file_mapper(const char* game_id, GmmFileMapperFn fn,
                   void* user_data = nullptr) {
    raw->register_file_mapper(raw, game_id, fn, user_data);
    return *this;
  }

  // -- Sort provider --
  Ctx& sort_provider(GmmSortFn sort_fn, void* user_data = nullptr) {
    raw->register_sort_provider(raw, sort_fn, user_data);
    return *this;
  }

  // -- Stage claim --
  Ctx& stage_claim(const char* stage_name, GmmStageFnV2 fn,
                   int priority = 100) {
    raw->register_stage_claim(raw, stage_name, fn, priority);
    return *this;
  }

  // -- Wildcard stage claim --
  Ctx& wildcard_stage_claim(const char* game_id, const char* stage_name,
                            GmmStageFnV2 fn, int priority = 100) {
    raw->register_wildcard_stage_claim(raw, game_id, stage_name, fn,
                                       priority);
    return *this;
  }

  // -- Preview --
  Ctx& preview(const char* file_extension, void* preview_data,
               GmmPreviewFn fn, void* user_data = nullptr) {
    raw->register_preview(raw, file_extension, preview_data, fn, user_data);
    return *this;
  }

  // -- Tool --
  Ctx& tool(const char* tool_id, const char* kind, GmmToolInvokeFn fn = nullptr,
            void* user_data = nullptr) {
    raw->register_tool(raw, tool_id, kind, fn, user_data);
    return *this;
  }

  // -- Mod page --
  Ctx& modpage(const char* url, GmmModPageDownloadFn fn,
               void* user_data = nullptr) {
    raw->register_modpage(raw, url, fn, user_data);
    return *this;
  }

  // -- Save parser --
  Ctx& save_parser(const char* game_id, GmmSaveParserFnV2 fn,
                   int priority = 100, void* user_data = nullptr) {
    raw->register_save_parser(raw, game_id, fn, priority, user_data);
    return *this;
  }

  // -- Animation parser --
  Ctx& animation_parser(const char* game_id, const char* file_extension,
                        GmmAnimationParserFnV2 fn, int priority = 100,
                        void* user_data = nullptr) {
    raw->register_animation_parser(raw, game_id, file_extension, fn, priority,
                                   user_data);
    return *this;
  }

  // -- Settings --
  Ctx& settings(std::initializer_list<const char*> keys,
                std::initializer_list<const char*> values) {
    raw->register_settings(raw, keys.begin(), values.begin(),
                           keys.size());
    return *this;
  }

  // -- Settings tab --
  Ctx& settings_tab(const char* title,
                    std::initializer_list<const char*> keys,
                    std::initializer_list<const char*> types,
                    std::initializer_list<const char*> defaults,
                    std::initializer_list<const char*> options) {
    raw->register_settings_tab(raw, title, keys.begin(), types.begin(),
                               defaults.begin(), options.begin(),
                               keys.size());
    return *this;
  }

  // -- Requirements --
  Ctx& requirements(GmmRequirementsFn fn, void* user_data = nullptr) {
    raw->register_requirements(raw, fn, user_data);
    return *this;
  }

  // -- Diagnostics --
  Ctx& diagnostics(const char* game_id, GmmDiagnoseFn fn,
                   void* user_data = nullptr) {
    raw->register_diagnostics(raw, game_id, fn, user_data);
    return *this;
  }
};

// -- Plugin base class for the declarative DX --
//
// Subclass this and override the virtuals you need, then use
// REGISTER_PLUGIN(MyPlugin) at the bottom of your .cpp.
//
// Example:
//   class SkyrimPlugin : public gmm::Plugin {
//   public:
//     GameInfo gameInfo() const override { return {...}; }
//     std::vector<Tab> tabs() const override { return {...}; }
//     void features(Ctx& ctx) const override { ctx.tool("loot", "advisory"); }
//     void hooks(Ctx& ctx) const override {
//       ctx.hook("steam_appid", "489830");
//     }
//   };
//   REGISTER_PLUGIN(SkyrimPlugin)
class Plugin {
public:
  virtual ~Plugin() = default;

  // Required: game identity
  virtual GmmGameInfo gameInfo() const = 0;

  // Optional: plugin metadata (name, author, version, description)
  virtual GmmPluginInfo pluginInfo() const { return {}; }

  // Optional: UI tabs
  virtual std::vector<Tab> tabs() const { return {}; }

  // Optional: categories
  virtual std::vector<Category> categories() const { return {}; }

  // Optional: features (tools, save parsers, etc.)
  virtual void features(Ctx& ctx) const {}

  // Optional: hooks (game-dependent key-value data)
  virtual void hooks(Ctx& ctx) const {}

  // Optional: event subscriptions
  virtual void events(Ctx& ctx) const {}
};

} // namespace gmm

// -- REGISTER_PLUGIN macro --
//
// Generates the two extern "C" entry points that the engine looks for.
// Usage: REGISTER_PLUGIN(MyPluginClass)
#define REGISTER_PLUGIN(T)                                                    \
  extern "C" void gmm_register_v2(GmmRegistrationCtxV2* raw) {               \
    if (!raw)                                                                \
      return;                                                                \
    gmm::Ctx ctx{raw};                                                       \
    T plugin;                                                                \
    auto gi = plugin.gameInfo();                                             \
    ctx.game(gi);                                                            \
    auto pi = plugin.pluginInfo();                                           \
    if (pi.name)                                                             \
      ctx.plugin(pi);                                                        \
    auto t = plugin.tabs();                                                  \
    if (!t.empty())                                                          \
      ctx.tabs({t.begin(), t.end()});                                        \
    auto c = plugin.categories();                                            \
    if (!c.empty())                                                          \
      ctx.categories({c.begin(), c.end()});                                  \
    plugin.features(ctx);                                                    \
    plugin.hooks(ctx);                                                       \
    plugin.events(ctx);                                                      \
  }                                                                          \
  extern "C" uint32_t gmm_abi_version(void) { return GMM_ABI_VERSION; }

#endif /* GMM_PLUGIN_HPP */
