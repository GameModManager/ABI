#ifndef GMM_DETAIL_THUNK_H
#define GMM_DETAIL_THUNK_H

#include <gmm_abi_v3.h>

#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <string_view>

namespace gmm {
namespace detail {

/* Args struct for the saves.parse capability. Constructed on the stack
 * inside the plugin; std::filesystem::path / std::string_view are built
 * here, never across the ABI boundary. */
struct SaveParseArgs {
  const char *path = nullptr;
  const char *game_id = nullptr;
  char err[256] = {};
};

} /* namespace detail */
} /* namespace gmm */

#endif /* GMM_DETAIL_THUNK_H */
