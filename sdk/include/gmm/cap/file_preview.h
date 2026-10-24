#ifndef GMM_CAP_FILE_PREVIEW_H
#define GMM_CAP_FILE_PREVIEW_H
#include "../capability.h"
#include "../version.h"
#include <gmm_abi_v3.h>
#include <QWidget>
#include <filesystem>
#include <string_view>
namespace gmm { namespace cap {
struct FilePreview : Capability {
  static constexpr const char *kId = GMM_CAPID_PREVIEW_FILE;
  const char *cap_id() const override { return kId; }
  virtual std::string_view extension() const = 0;
  virtual QWidget *create(const std::filesystem::path &file) = 0;
};
template <class T>
int32_t file_preview_thunk(void *self, uint32_t method, void *args, void *out) {
  auto &obj = *static_cast<T *>(self);
  try {
    if (method == GMM_PREVIEW_CREATE) {
      *static_cast<QWidget **>(out) = obj.create(*static_cast<std::filesystem::path *>(args));
      return GMM_OK;
    }
    return GMM_UNSUPPORTED;
  } catch (...) { return GMM_ERR; }
}
} }
#endif
