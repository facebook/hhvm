<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Network;

/** General behavior for selecting which IP version to use.
 *
 * Use `IPProtocolVersion` instead if a specific version is required.
 */
enum IPProtocolBehavior: int {
  PREFER_IPV6 = 0;
  FORCE_IPV6 = 6;
  FORCE_IPV4 = 4;
}
