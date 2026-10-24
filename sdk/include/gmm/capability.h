#ifndef GMM_CAPABILITY_H
#define GMM_CAPABILITY_H

#include "version.h"

#include <string_view>

namespace gmm {

class Capability {
public:
  virtual ~Capability() = default;
  virtual const char *cap_id() const = 0;
  virtual int priority() const { return 0; }
};

} /* namespace gmm */

#endif /* GMM_CAPABILITY_H */
