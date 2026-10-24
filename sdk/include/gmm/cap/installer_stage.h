#ifndef GMM_CAP_INSTALLER_STAGE_H
#define GMM_CAP_INSTALLER_STAGE_H
#include "../capability.h"
#include "../version.h"
#include <gmm_abi_v3.h>
#include <string_view>
namespace gmm { namespace cap {
struct StageContext {};
struct StageClaim : Capability {
  static constexpr const char *kId = GMM_CAPID_INSTALLER_STAGE;
  const char *cap_id() const override { return kId; }
  virtual std::string_view stage_name() const = 0;
  virtual bool install(StageContext &) = 0;
};
template <class T>
int32_t installer_stage_thunk(void *self, uint32_t method, void *args, void *out) {
  return (method == GMM_STAGE_INSTALL) ? GMM_OK : GMM_UNSUPPORTED;
}
} }
#endif
