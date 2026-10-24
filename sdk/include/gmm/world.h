#ifndef GMM_WORLD_H
#define GMM_WORLD_H

#include "capability.h"
#include "game.h"
#include "version.h"

#include "cap/animation_parser.h"
#include "cap/detection_validator.h"
#include "cap/event_subscriber.h"
#include "cap/file_preview.h"
#include "cap/installer_stage.h"
#include "cap/load_order.h"
#include "cap/save_overlay.h"
#include "cap/save_parser.h"
#include "cap/sort_provider.h"
#include "cap/tool.h"
#include "cap/ui_diagnostics.h"
#include "cap/ui_settings.h"

#include <gmm_abi_v3.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace gmm {

class Plugin;

class World {
public:
  explicit World(const GmmHostServices &host) : host_(host) {}

  void add(const GameDesc &desc) {
    GmmGameEntity ge{};
    ge.version = 1;
    ge.game_id = dup(desc.id);
    ge.display_name = dup(desc.name);

    if (!desc.steam_appids.empty()) {
      std::uint32_t *arr = new std::uint32_t[desc.steam_appids.size()];
      std::copy(desc.steam_appids.begin(), desc.steam_appids.end(), arr);
      ge.steam_appids = arr;
      ge.steam_appid_count = static_cast<std::uint32_t>(desc.steam_appids.size());
    }
    ge.gog_id = dup(desc.gog_id);
    ge.epic_namespace = dup(desc.epic_namespace);
    ge.nexus_domain = dup(desc.nexus_domain);

    if (!desc.aliases.empty()) {
      const char **arr = new const char *[desc.aliases.size()];
      for (std::size_t i = 0; i < desc.aliases.size(); ++i)
        arr[i] = dup(desc.aliases[i]);
      ge.aliases = arr;
      ge.alias_count = static_cast<std::uint32_t>(desc.aliases.size());
    }
    ge.mods_subpath = dup(desc.mods_dir);

    if (!desc.executables.empty()) {
      const char **arr = new const char *[desc.executables.size()];
      for (std::size_t i = 0; i < desc.executables.size(); ++i)
        arr[i] = dup(desc.executables[i]);
      ge.executables = arr;
      ge.executable_count = static_cast<std::uint32_t>(desc.executables.size());
    }

    game_entities_.push_back(ge);
  }

  template <class C, class... A>
  void attach(std::string_view game_id, A &&...args) {
    auto obj = std::make_unique<C>(std::forward<A>(args)...);
    Capability *raw = obj.get();

    GmmCapabilityObject co{};
    co.id = C::kId;
    co.game_id = dup(std::string(game_id));
    co.version = 1;
    co.priority = raw->priority();
    co.self = raw;
    co.invoke = invoke_for<C>();

    cap_objs_.push_back(co);
    caps_.push_back(std::move(obj));
  }

  template <class C, class... A>
  void attach_app(A &&...args) {
    attach<C>("", std::forward<A>(args)...);
  }

  const GmmHostServices &host() const { return host_; }

  void set_plugin_info(const char *name, const char *author, const char *version_string,
                       const char *description) {
    plugin_name_ = dup_str(name);
    plugin_author_ = dup_str(author);
    plugin_version_ = dup_str(version_string);
    plugin_description_ = dup_str(description);
  }

  const GmmPluginDesc *seal() {
    GmmPluginDesc *desc = new GmmPluginDesc{};
    desc->version = GMM_API_VERSION;
    desc->name = plugin_name_ ? plugin_name_
                              : (game_entities_.empty() ? nullptr : game_entities_[0].display_name);
    desc->author = plugin_author_;
    desc->version_string = plugin_version_;
    desc->description = plugin_description_;
    if (!game_entities_.empty()) {
      desc->games = game_entities_.data();
      desc->game_count = static_cast<std::uint32_t>(game_entities_.size());
    }
    if (!cap_objs_.empty()) {
      desc->caps = cap_objs_.data();
      desc->cap_count = static_cast<std::uint32_t>(cap_objs_.size());
    }
    return desc;
  }

private:
  static char *dup(const char *s) {
    if (!s || !*s)
      return nullptr;
    std::size_t n = std::strlen(s);
    char *p = new char[n + 1];
    std::copy(s, s + n, p);
    p[n] = '\0';
    return p;
  }
  static char *dup(const std::string &s) {
    if (s.empty())
      return nullptr;
    char *p = new char[s.size() + 1];
    std::copy(s.begin(), s.end(), p);
    p[s.size()] = '\0';
    return p;
  }
  static const char *dup_str(const char *s) { return dup(s); }

  template <class C>
  static GmmInvokeFn invoke_for() {
    if constexpr (std::is_base_of_v<cap::SaveParser, C>)
      return &cap::save_parser_thunk<C>;
    if constexpr (std::is_base_of_v<cap::SaveOverlay, C>)
      return &cap::save_overlay_thunk<C>;
#if GMM_HAS_QT
    if constexpr (std::is_base_of_v<cap::FilePreview, C>)
      return &cap::file_preview_thunk<C>;
#endif
    if constexpr (std::is_base_of_v<cap::AnimationParser, C>)
      return &cap::animation_parser_thunk<C>;
    if constexpr (std::is_base_of_v<cap::SortProvider, C>)
      return &cap::sort_provider_thunk<C>;
    if constexpr (std::is_base_of_v<cap::Tool, C>)
      return &cap::tool_thunk<C>;
    if constexpr (std::is_base_of_v<cap::StageClaim, C>)
      return &cap::installer_stage_thunk<C>;
    if constexpr (std::is_base_of_v<cap::DetectionValidator, C>)
      return &cap::detection_validator_thunk<C>;
    if constexpr (std::is_base_of_v<cap::EventSubscriber, C>)
      return &cap::event_subscriber_thunk<C>;
    if constexpr (std::is_base_of_v<cap::Diagnostics, C>)
      return &cap::ui_diagnostics_thunk<C>;
    if constexpr (std::is_base_of_v<cap::UiSettings, C>)
      return &cap::ui_settings_thunk<C>;
    if constexpr (std::is_base_of_v<cap::OrderEncoding, C>)
      return &cap::load_order_thunk<C>;
    return nullptr;
  }

  GmmHostServices host_;
  std::vector<GameDesc> games_desc_;
  std::vector<std::unique_ptr<Capability>> caps_;
  std::vector<GmmCapabilityObject> cap_objs_;
  std::vector<GmmGameEntity> game_entities_;

  const char *plugin_name_ = nullptr;
  const char *plugin_author_ = nullptr;
  const char *plugin_version_ = nullptr;
  const char *plugin_description_ = nullptr;
};

} /* namespace gmm */

#endif /* GMM_WORLD_H */
