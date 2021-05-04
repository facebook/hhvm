<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\TCP;

use namespace HH\Lib\{IO, Network};

/**
 * A TCP client or server socket.
 *
 * @see `TCP\Server` to create a server.
 * @see `TCP\connect_async()` to connect to an existing server.
 */
<<__Sealed(CloseableSocket::class)>>
interface Socket extends Network\Socket {
  /** A host and port number */
  const type TAddress = (string, int);
}
