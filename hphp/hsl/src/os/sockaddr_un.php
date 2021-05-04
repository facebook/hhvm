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

/** Address of a UNIX-domain socket.
 *
 * UNIX sockets *may* have a path, which will usually - but not always - exist
 * on the local filesystem.
 *
 * See `man 7 unix` (Linux) or `man 6 unix` (BSD) for details.
 */
final class sockaddr_un extends sockaddr {
  public function __construct(private ?string $path) {}

  <<__Override>>
  final public function getFamily(): AddressFamily {
    return AddressFamily::AF_UNIX;
  }

  /** Get the path (if any) of a socket.
   *
   * @returns `null` if the socket does not have a path, for example, if created
   *   with `socketpair()`
   * @returns a `string` if the socket does have a path; this is usually - but
   *   not always - a filesystem path. For example, Linux supports 'abstract'
   *   unix sockets, which have a path beginning with a null byte and do not
   *   correspond to the filesystem.
   */
  final public function getPath(): ?string {
    return $this->path;
  }
}
