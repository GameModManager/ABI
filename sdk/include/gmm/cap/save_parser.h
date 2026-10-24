#ifndef GMM_CAP_SAVE_PARSER_H
#define GMM_CAP_SAVE_PARSER_H

#include "../capability.h"
#include "../detail/thunk.h"
#include "../version.h"

#include <gmm_abi_v2.h>
#include <gmm_abi_v3.h>

#include <cstdio>
#include <filesystem>
#include <string_view>

namespace gmm {
namespace cap {

struct SaveParser : Capability {
  static constexpr const char *kId = GMM_CAPID_SAVES_PARSE;
  const char *cap_id() const override { return kId; }
  virtual void parse(const std::filesystem::path &, std::string_view game_id,
                     GmmSaveDataV2 &out) = 0;
  virtual bool parse_fast(const std::filesystem::path &, std::string_view,
                          GmmSaveDataV2 &) {
    return false;
  }
};

template <class T>
int32_t save_parser_thunk(void *self, uint32_t method, void *args, void *out) {
  auto *a = static_cast<detail::SaveParseArgs *>(args);
  auto &obj = *static_cast<T *>(self);
  try {
    std::filesystem::path p = a && a->path ? a->path : "";
    std::string_view gid = a && a->game_id ? a->game_id : "";
    switch (method) {
    case GMM_SAVES_PARSE_FULL:
      obj.parse(p, gid, *static_cast<GmmSaveDataV2 *>(out));
      return GMM_OK;
    case GMM_SAVES_PARSE_FAST:
      return obj.parse_fast(p, gid, *static_cast<GmmSaveDataV2 *>(out)) ? GMM_OK
                                                                       : GMM_UNSUPPORTED;
    default:
      return GMM_UNSUPPORTED;
    }
  } catch (const std::exception &e) {
    if (a)
      std::snprintf(a->err, sizeof(a->err), "%s", e.what());
    return GMM_ERR;
  } catch (...) {
    return GMM_ERR;
  }
}

} /* namespace cap */
} /* namespace gmm */

#endif /* GMM_CAP_SAVE_PARSER_H */
