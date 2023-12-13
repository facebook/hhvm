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
      status5xx(name + "5xx", SUM) {
  status39x.emplace(name + "39x", SUM);
  status200.emplace(name + "200", SUM);
  status206.emplace(name + "206", SUM);
  status301.emplace(name + "301", SUM);
  status302.emplace(name + "302", SUM);
  status303.emplace(name + "303", SUM);
  status304.emplace(name + "304", SUM);
  status307.emplace(name + "307", SUM);
  status395.emplace(name + "395", SUM);
  status396.emplace(name + "396", SUM);
  status397.emplace(name + "397", SUM);
  status398.emplace(name + "398", SUM);
  status399.emplace(name + "399", SUM);
  status400.emplace(name + "400", SUM);
  status401.emplace(name + "401", SUM);
  status403.emplace(name + "403", SUM);
  status404.emplace(name + "404", SUM);
  status408.emplace(name + "408", SUM);
  status429.emplace(name + "429", SUM);
  status500.emplace(name + "500", SUM);
  status501.emplace(name + "501", SUM);
  status502.emplace(name + "502", SUM);
  status503.emplace(name + "503", SUM);
  status504.emplace(name + "504", SUM);
}

void TLResponseCodeStats::addStatus(int status) {
  switch (status) {
    case 200:
      status2xx.add(1);
      BaseStats::addToOptionalStat(status200, 1);
      return;
    case 404:
      status4xx.add(1);
      BaseStats::addToOptionalStat(status404, 1);
      return;
    case 206:
      status2xx.add(1);
      BaseStats::addToOptionalStat(status206, 1);
      return;
    case 301:
      status3xx.add(1);
      BaseStats::addToOptionalStat(status301, 1);
      return;
    case 302:
      status3xx.add(1);
      BaseStats::addToOptionalStat(status302, 1);
      return;
    case 303:
      status3xx.add(1);
      BaseStats::addToOptionalStat(status303, 1);
      return;
    case 304:
      status3xx.add(1);
      BaseStats::addToOptionalStat(status304, 1);
      return;
    case 307:
      status3xx.add(1);
      BaseStats::addToOptionalStat(status307, 1);
      return;
    case 390:
    case 391:
    case 392:
    case 393:
    case 394:
      status3xx.add(1);
      BaseStats::addToOptionalStat(status39x, 1);
      return;
    case 395:
      status3xx.add(1);
      BaseStats::addToOptionalStat(status39x, 1);
      BaseStats::addToOptionalStat(status395, 1);
      return;
    case 396:
      status3xx.add(1);
      BaseStats::addToOptionalStat(status39x, 1);
      BaseStats::addToOptionalStat(status396, 1);
      return;
    case 397:
      status3xx.add(1);
      BaseStats::addToOptionalStat(status39x, 1);
      BaseStats::addToOptionalStat(status397, 1);
      return;
    case 398:
      status3xx.add(1);
      BaseStats::addToOptionalStat(status39x, 1);
      BaseStats::addToOptionalStat(status398, 1);
      return;
    case 399:
      status3xx.add(1);
      BaseStats::addToOptionalStat(status39x, 1);
      BaseStats::addToOptionalStat(status399, 1);
      return;
    case 400:
      status4xx.add(1);
      BaseStats::addToOptionalStat(status400, 1);
      return;
    case 401:
      status4xx.add(1);
      BaseStats::addToOptionalStat(status401, 1);
      return;
    case 403:
      status4xx.add(1);
      BaseStats::addToOptionalStat(status403, 1);
      return;
    case 408:
      status4xx.add(1);
      BaseStats::addToOptionalStat(status408, 1);
      return;
    case 429:
      status4xx.add(1);
      BaseStats::addToOptionalStat(status429, 1);
      return;
    case 500:
      status5xx.add(1);
      BaseStats::addToOptionalStat(status500, 1);
      return;
    case 501:
      status5xx.add(1);
      BaseStats::addToOptionalStat(status501, 1);
      return;
    case 502:
      status5xx.add(1);
      BaseStats::addToOptionalStat(status502, 1);
      return;
    case 503:
      status5xx.add(1);
      BaseStats::addToOptionalStat(status503, 1);
      return;
    case 504:
      status5xx.add(1);
      BaseStats::addToOptionalStat(status504, 1);
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
