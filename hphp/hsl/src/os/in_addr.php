<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\OS;

/** The type of the network form of an INET (IPv4) address, in host byte order.
 *
 * Note that this differs from the C API, which uses network byte order.
 *
 * @see `in6_addr`
 */
type in_addr = int;
