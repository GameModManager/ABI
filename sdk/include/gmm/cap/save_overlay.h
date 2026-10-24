#ifndef GMM_CAP_SAVE_OVERLAY_H
#define GMM_CAP_SAVE_OVERLAY_H
#include "../capability.h"
#include "../version.h"
#include <gmm_abi_v2.h>
#include <gmm_abi_v3.h>
namespace gmm { namespace cap {
struct SaveOverlay : Capability {
  static constexpr const char *kId = GMM_CAPID_SAVES_OVERLAY;
  const char *cap_id() const override { return kId; }
  virtual GmmSaveOverlayV2 *build(const GmmSaveDataV2 &) = 0;
};
template <class T>
int32_t save_overlay_thunk(void *self, uint32_t method, void *args, void *out) {
  auto &obj = *static_cast<T *>(self);
  try {
    if (method == GMM_OVERLAY_BUILD) {
      *static_cast<GmmSaveOverlayV2 **>(out) = obj.build(*static_cast<GmmSaveDataV2 *>(args));
      return GMM_OK;
    }
    return GMM_UNSUPPORTED;
  } catch (...) { return GMM_ERR; }
}
} }
#endif
