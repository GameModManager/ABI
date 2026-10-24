#ifndef GMM_CAP_TOOL_H
#define GMM_CAP_TOOL_H
#include "../capability.h"
#include "../version.h"
#include <gmm_abi_v3.h>
#include <string_view>
namespace gmm { namespace cap {
struct Tool : Capability {
  static constexpr const char *kId = GMM_CAPID_TOOL;
  const char *cap_id() const override { return kId; }
  virtual std::string_view tool_id() const = 0;
  virtual std::string_view kind() const { return "advisory"; }
  virtual void invoke() {}
};
template <class T>
int32_t tool_thunk(void *self, uint32_t method, void *args, void *out) {
  auto &obj = *static_cast<T *>(self);
  try {
    if (method == GMM_TOOL_INVOKE) { obj.invoke(); return GMM_OK; }
    return GMM_UNSUPPORTED;
  } catch (...) { return GMM_ERR; }
}
} }
#endif
