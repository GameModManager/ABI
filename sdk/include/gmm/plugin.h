#ifndef GMM_PLUGIN_H
#define GMM_PLUGIN_H

#if defined(GMM_PLUGIN_HPP)
#error "plugin.hpp (API 2) and gmm/plugin.h (API 3) cannot be mixed in one TU"
#endif

#include "version.h"
#include "world.h"

#include <string>
#include <string_view>

namespace gmm {

class Plugin {
public:
  virtual ~Plugin() = default;
  virtual std::string_view name() const = 0;
  virtual std::string_view author() const { return {}; }
  virtual std::string_view version() const { return {}; }
  virtual std::string_view description() const { return {}; }
  virtual void build(World &world) = 0;
};

} /* namespace gmm */

#define GMM_PLUGIN(Impl)                                                          \
  extern "C" uint32_t gmm_abi_version(void) { return GMM_API_VERSION; }           \
  extern "C" const GmmPluginDesc *gmm_entry(const GmmHostServices *h) {           \
    if (!h)                                                                       \
      return nullptr;                                                             \
    auto *w = new gmm::World(*h);                                                 \
    Impl plugin;                                                                  \
    plugin.build(*w);                                                             \
    w->set_plugin_info(plugin.name().data(), plugin.author().data(),               \
                       plugin.version().data(), plugin.description().data());      \
    return w->seal();                                                             \
  }

#endif /* GMM_PLUGIN_H */
