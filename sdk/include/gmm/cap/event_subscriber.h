#ifndef GMM_CAP_EVENT_SUBSCRIBER_H
#define GMM_CAP_EVENT_SUBSCRIBER_H
#include "../capability.h"
#include "../version.h"
#include <gmm_abi_v3.h>
#include <string>
#include <string_view>
#include <vector>
namespace gmm { namespace cap {
struct EventSubscriber : Capability {
  static constexpr const char *kId = GMM_CAPID_EVENT_SUBSCRIBE;
  const char *cap_id() const override { return kId; }
  virtual std::vector<std::string> events() const = 0;
  virtual void on_event(std::string_view event, std::string_view json) = 0;
};
template <class T>
int32_t event_subscriber_thunk(void *self, uint32_t method, void *args, void *out) {
  return (method == GMM_EVENT_DELIVER) ? GMM_OK : GMM_UNSUPPORTED;
}
} }
#endif
