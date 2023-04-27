/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/stats/ResponseCodeStatsMinute.h>

using facebook::fb303::COUNT;

namespace proxygen {

ResponseCodeStatsMinute::ResponseCodeStatsMinute(const std::string& name)
    : statusOther(name + "other", COUNT),
      status1xx(name + "1xx", COUNT),
      status2xx(name + "2xx", COUNT),
      status3xx(name + "3xx", COUNT),
      status4xx(name + "4xx", COUNT),
      status5xx(name + "5xx", COUNT) {
}

void ResponseCodeStatsMinute::addStatus(int status) {
  if (status < 100) {
    statusOther.add(1);
  } else if (status < 200) {
    status1xx.add(1);
  } else if (status < 300) {
    status2xx.add(1);
  } else if (status < 400) {
    status3xx.add(1);
  } else if (status < 500) {
    status4xx.add(1);
  } else if (status < 600) {
    status5xx.add(1);
  } else {
    statusOther.add(1);
  }
}

} // namespace proxygen
