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

/** A specific version of IP.
 *
 * Use `IPProtocolBehavior` instead if possible.
 */
enum IPProtocolVersion : int {
  IPV6 = 6;
  IPV4 = 4;
}
