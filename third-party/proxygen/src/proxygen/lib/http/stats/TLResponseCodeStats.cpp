/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/stats/TLResponseCodeStats.h>

using facebook::fb303::SUM;

namespace proxygen {

TLResponseCodeStats::TLResponseCodeStats(const std::string& name)
    : statusNone(name + "nostatus", SUM),
      statusOther(name + "other", SUM),
      status1xx(name + "1xx", SUM),
      status2xx(name + "2xx", SUM),
      status3xx(name + "3xx", SUM),
      status4xx(name + "4xx", SUM),
      status5xx(name + "5xx", SUM),
      status39x(name + "39x", SUM),
      status200(name + "200", SUM),
      status206(name + "206", SUM),
      status301(name + "301", SUM),
      status302(name + "302", SUM),
      status303(name + "303", SUM),
      status304(name + "304", SUM),
      status307(name + "307", SUM),
      status395(name + "395", SUM),
      status396(name + "396", SUM),
      status397(name + "397", SUM),
      status398(name + "398", SUM),
      status399(name + "399", SUM),
      status400(name + "400", SUM),
      status401(name + "401", SUM),
      status403(name + "403", SUM),
      status404(name + "404", SUM),
      status408(name + "408", SUM),
      status429(name + "429", SUM),
      status500(name + "500", SUM),
      status501(name + "501", SUM),
      status502(name + "502", SUM),
      status503(name + "503", SUM),
      status504(name + "504", SUM) {
}

void TLResponseCodeStats::addStatus(int status) {
  switch (status) {
    case 200:
      status2xx.add(1);
      status200.add(1);
      return;
    case 404:
      status4xx.add(1);
      status404.add(1);
      return;
    case 206:
      status2xx.add(1);
      status206.add(1);
      return;
    case 301:
      status3xx.add(1);
      status301.add(1);
      return;
    case 302:
      status3xx.add(1);
      status302.add(1);
      return;
    case 303:
      status3xx.add(1);
      status303.add(1);
      return;
    case 304:
      status3xx.add(1);
      status304.add(1);
      return;
    case 307:
      status3xx.add(1);
      status307.add(1);
      return;
    case 390:
    case 391:
    case 392:
    case 393:
    case 394:
      status3xx.add(1);
      status39x.add(1);
      return;
    case 395:
      status3xx.add(1);
      status39x.add(1);
      status395.add(1);
      return;
    case 396:
      status3xx.add(1);
      status39x.add(1);
      status396.add(1);
      return;
    case 397:
      status3xx.add(1);
      status39x.add(1);
      status397.add(1);
      return;
    case 398:
      status3xx.add(1);
      status39x.add(1);
      status398.add(1);
      return;
    case 399:
      status3xx.add(1);
      status39x.add(1);
      status399.add(1);
      return;
    case 400:
      status4xx.add(1);
      status400.add(1);
      return;
    case 401:
      status4xx.add(1);
      status401.add(1);
      return;
    case 403:
      status4xx.add(1);
      status403.add(1);
      return;
    case 408:
      status4xx.add(1);
      status408.add(1);
      return;
    case 429:
      status4xx.add(1);
      status429.add(1);
      return;
    case 500:
      status5xx.add(1);
      status500.add(1);
      return;
    case 501:
      status5xx.add(1);
      status501.add(1);
      return;
    case 502:
      status5xx.add(1);
      status502.add(1);
      return;
    case 503:
      status5xx.add(1);
      status503.add(1);
      return;
    case 504:
      status5xx.add(1);
      status504.add(1);
      return;
    case 555:
      // 555 is returned on healthcheck failures. Skip counting it in exported
      // stats.
      return;
    default:
      break;
  }

  if (status < 0) {
    statusNone.add(1);
  } else if (status < 100) {
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
