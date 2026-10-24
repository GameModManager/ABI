#ifndef GMM_CAP_LOAD_ORDER_H
#define GMM_CAP_LOAD_ORDER_H
#include "../capability.h"
#include "../version.h"
#include <gmm_abi_v3.h>
#include <filesystem>
#include <string>
#include <vector>
namespace gmm { namespace cap {
struct OrderEncoding : Capability {
  static constexpr const char *kId = GMM_CAPID_LOAD_ORDER;
  const char *cap_id() const override { return kId; }
  virtual bool write(const std::vector<std::string> &mod_ids, const std::filesystem::path &out) = 0;
};
template <class T>
int32_t load_order_thunk(void *self, uint32_t method, void *args, void *out) {
  return (method == GMM_ORDER_WRITE) ? GMM_OK : GMM_UNSUPPORTED;
}
} }
#endif
