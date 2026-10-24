#ifndef GMM_GAME_H
#define GMM_GAME_H

#ifdef linux
#undef linux
#endif
#ifdef unix
#undef unix
#endif

#include "version.h"

#include <cstdint>
#include <filesystem>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace gmm {

template <class T>
struct ByTarget {
  T windows{};
  T linux{};
  T macos{};
  T default_{};
};

struct Saves {
  std::vector<const char *> extensions;
  const char *subdir = nullptr;
};

struct Paths {
  ByTarget<const char *> saves;
};

struct Catalog {
  const char *icon_url = nullptr;
  std::vector<const char *> download_sources;
  const char *masterlist_url = nullptr;
};

struct InstallGlob {
  std::vector<const char *> folders;
  std::vector<const char *> files;
};

struct InstallMove {
  const char *from = nullptr;
  const char *to = nullptr;
};

struct InstallMoveList {
  std::vector<InstallMove> folders;
  std::vector<InstallMove> files;
};

struct InstallChecker {
  InstallGlob valid;
  InstallGlob ignore;
  InstallGlob delete_;
  InstallMoveList move;
};

struct Install {
  InstallChecker checker;
};

struct PluginSettingEntry {
  const char *key = nullptr;
  const char *type = nullptr;
  const char *default_value = nullptr;
  std::vector<const char *> options;
};

struct PluginSettings {
  const char *title = nullptr;
  std::vector<PluginSettingEntry> entries;
};

struct DataCap {
  const char *id = nullptr;
  std::vector<const char *> dirs;
  std::vector<const char *> extensions;
};

struct GameDesc {
  const char *id = nullptr;
  const char *name = nullptr;
  std::vector<std::uint32_t> steam_appids;
  const char *gog_id = nullptr;
  const char *epic_namespace = nullptr;
  const char *nexus_domain = nullptr;
  std::vector<const char *> aliases;
  const char *author = nullptr;
  const char *description = nullptr;
  ByTarget<const char *> binary;
  ByTarget<const char *> launcher;
  std::vector<const char *> executables;
  const char *mods_dir = nullptr;
  std::vector<const char *> valid_dirs;
  std::vector<const char *> valid_exts;
  bool case_sensitive = false;
  std::vector<const char *> native_files;
  Paths paths;
  Saves saves;
  Catalog catalog;
  std::vector<DataCap> capabilities;
  Install install;
  PluginSettings settings;
};

} /* namespace gmm */

#endif /* GMM_GAME_H */
