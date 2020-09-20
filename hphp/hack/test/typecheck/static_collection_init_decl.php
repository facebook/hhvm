<?hh // partial
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class CacheConsistencyRequestLog {
  private static Vector<array> $taoRequestLog = Vector {};
  private static Vector<Map<string, mixed>> $memcacheRequestLog = Vector {};
}
