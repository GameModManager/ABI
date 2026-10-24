#ifndef GMM_BASIC_GAME_H
#define GMM_BASIC_GAME_H

#include "game.h"
#include "version.h"
#include "world.h"

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace gmm {

struct BasicGame {
  virtual ~BasicGame() = default;
  virtual void attach(World &) const {}

  template <class Self>
  static void describe(const Self &self, World &w) {
    static_assert(requires { self.id; }, "BasicGame: missing required field 'id'");
    static_assert(requires { self.name; }, "BasicGame: missing required field 'name'");
    static_assert(requires { self.mods_dir; }, "BasicGame: missing required field 'mods_dir'");

    GameDesc desc{};
    desc.id = self.id;
    desc.name = self.name;
    if constexpr (requires { self.mods_dir; })
      desc.mods_dir = self.mods_dir;
    if constexpr (requires { self.steam_appids; })
      desc.steam_appids = self.steam_appids;
    if constexpr (requires { self.gog_id; })
      desc.gog_id = self.gog_id;
    if constexpr (requires { self.epic_namespace; })
      desc.epic_namespace = self.epic_namespace;
    if constexpr (requires { self.nexus_domain; })
      desc.nexus_domain = self.nexus_domain;
    if constexpr (requires { self.aliases; })
      desc.aliases = self.aliases;
    if constexpr (requires { self.author; })
      desc.author = self.author;
    if constexpr (requires { self.description; })
      desc.description = self.description;
    if constexpr (requires { self.binary; })
      desc.binary = self.binary;
    if constexpr (requires { self.launcher; })
      desc.launcher = self.launcher;
    if constexpr (requires { self.executables; })
      desc.executables = self.executables;
    if constexpr (requires { self.valid_dirs; })
      desc.valid_dirs = self.valid_dirs;
    if constexpr (requires { self.valid_exts; })
      desc.valid_exts = self.valid_exts;
    if constexpr (requires { self.case_sensitive; })
      desc.case_sensitive = self.case_sensitive;
    if constexpr (requires { self.native_files; })
      desc.native_files = self.native_files;
    if constexpr (requires { self.paths; })
      desc.paths = self.paths;
    if constexpr (requires { self.saves; })
      desc.saves = self.saves;
    if constexpr (requires { self.catalog; })
      desc.catalog = self.catalog;
    if constexpr (requires { self.capabilities; })
      desc.capabilities = self.capabilities;
    if constexpr (requires { self.install; })
      desc.install = self.install;
    if constexpr (requires { self.settings; })
      desc.settings = self.settings;

    w.add(desc);
  }
};

} /* namespace gmm */

#define GMM_BASIC_GAME(T)                                                         \
  extern "C" uint32_t gmm_abi_version(void) { return GMM_API_VERSION; }           \
  extern "C" const GmmPluginDesc *gmm_entry(const GmmHostServices *h) {           \
    if (!h)                                                                       \
      return nullptr;                                                             \
    auto *w = new gmm::World(*h);                                                 \
    T game;                                                                       \
    gmm::BasicGame::describe(game, *w);                                           \
    game.attach(*w);                                                              \
    w->set_plugin_info(game.id, "GameModManager Team", nullptr, nullptr);          \
    return w->seal();                                                             \
  }

#endif /* GMM_BASIC_GAME_H */
