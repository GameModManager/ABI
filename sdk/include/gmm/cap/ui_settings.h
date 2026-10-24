#ifndef GMM_CAP_UI_SETTINGS_H
#define GMM_CAP_UI_SETTINGS_H
#include "../capability.h"
#include "../version.h"
#include <gmm_abi_v3.h>
#include <string_view>
namespace gmm { namespace cap {
struct UiSettings : Capability {
  static constexpr const char *kId = GMM_CAPID_UI_SETTINGS;
  const char *cap_id() const override { return kId; }
  virtual std::string_view title() const { return {}; }
};
template <class T>
int32_t ui_settings_thunk(void *self, uint32_t method, void *args, void *out) {
  return GMM_OK;
}
} }
#endif
