#ifndef GMM_CAP_ANIMATION_PARSER_H
#define GMM_CAP_ANIMATION_PARSER_H
#include "../capability.h"
#include "../version.h"
#include <gmm_abi_v2.h>
#include <gmm_abi_v3.h>
#include <filesystem>
#include <string_view>
namespace gmm { namespace cap {
struct AnimationParser : Capability {
  static constexpr const char *kId = GMM_CAPID_ANIMATION_PARSE;
  const char *cap_id() const override { return kId; }
  virtual std::string_view extension() const = 0;
  virtual bool parse(const std::filesystem::path &file, const std::filesystem::path &base_dir,
                    GmmAnimationDataV2 &out) = 0;
};
template <class T>
int32_t animation_parser_thunk(void *self, uint32_t method, void *args, void *out) {
  return (method == GMM_ANIM_PARSE) ? GMM_OK : GMM_UNSUPPORTED;
}
} }
#endif
