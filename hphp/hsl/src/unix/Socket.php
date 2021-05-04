<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\Unix;

use namespace HH\Lib\{IO, Network};

/** A Unix socket for a server or client connection.
 *
 * @see `Unix\Server` to accept new connections
 * @see `Unix\connect_async()` to connect to an existing server
 */
<<__Sealed(CloseableSocket::class)>>
interface Socket extends Network\Socket {
  /** An identifier; usually a file path, but this isn't guaranteed. */
  const type TAddress = ?string;
}
