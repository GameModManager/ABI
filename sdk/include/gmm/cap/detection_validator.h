#ifndef GMM_CAP_DETECTION_VALIDATOR_H
#define GMM_CAP_DETECTION_VALIDATOR_H
#include "../capability.h"
#include "../version.h"
#include <gmm_abi_v3.h>
#include <filesystem>
namespace gmm { namespace cap {
struct DetectionValidator : Capability {
  static constexpr const char *kId = GMM_CAPID_DETECTION_VALIDATOR;
  const char *cap_id() const override { return kId; }
  virtual bool looks_valid(const std::filesystem::path &game_dir) = 0;
};
template <class T>
int32_t detection_validator_thunk(void *self, uint32_t method, void *args, void *out) {
  auto &obj = *static_cast<T *>(self);
  try {
    if (method == GMM_VALIDATE_DIR) {
      *static_cast<char *>(out) = obj.looks_valid(*static_cast<std::filesystem::path *>(args)) ? 1 : 0;
      return GMM_OK;
    }
    return GMM_UNSUPPORTED;
  } catch (...) { return GMM_ERR; }
}
} }
#endif
