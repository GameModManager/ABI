#ifndef GMM_CAP_UI_DIAGNOSTICS_H
#define GMM_CAP_UI_DIAGNOSTICS_H
#include "../capability.h"
#include "../version.h"
#include <gmm_abi_v3.h>
#include <string>
#include <vector>
namespace gmm { namespace cap {
struct Problem { std::string short_text; std::string full_text; std::string guided_fix; };
struct Diagnostics : Capability {
  static constexpr const char *kId = GMM_CAPID_UI_DIAGNOSTICS;
  const char *cap_id() const override { return kId; }
  virtual std::vector<Problem> collect() = 0;
};
template <class T>
int32_t ui_diagnostics_thunk(void *self, uint32_t method, void *args, void *out) {
  return (method == GMM_DIAGNOSE_COLLECT) ? GMM_OK : GMM_UNSUPPORTED;
}
} }
#endif
