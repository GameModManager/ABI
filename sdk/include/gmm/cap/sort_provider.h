#ifndef GMM_CAP_SORT_PROVIDER_H
#define GMM_CAP_SORT_PROVIDER_H
#include "../capability.h"
#include "../version.h"
#include <gmm_abi_v3.h>
#include <string>
#include <vector>
namespace gmm { namespace cap {
struct SortProvider : Capability {
  static constexpr const char *kId = GMM_CAPID_SORT_PROVIDER;
  const char *cap_id() const override { return kId; }
  virtual std::vector<std::string> sort(const std::vector<std::string> &mod_folders) = 0;
};
template <class T>
int32_t sort_provider_thunk(void *self, uint32_t method, void *args, void *out) {
  return (method == GMM_SORT_RUN) ? GMM_OK : GMM_UNSUPPORTED;
}
} }
#endif
