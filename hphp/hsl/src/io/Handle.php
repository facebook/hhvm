<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\IO;

use namespace HH\Lib\{File, Network};

/** An interface for an IO stream.
 *
 * For example, an IO handle might be attached to a file, a network socket, or
 * just an in-memory buffer.
 *
 * HSL IO handles can be thought of as having a combination of behaviors - some
 * of which are mutually exclusive - which are reflected in more-specific
 * interfaces; for example:
 * - Closeable
 * - Seekable
 * - Readable
 * - Writable
 *
 * These can be combined to arbitrary interfaces; for example, if you are
 * writing a function that writes some data, you may want to take a
 * `IO\WriteHandle` - or, if you read, write, and seek,
 * `IO\SeekableReadWriteHandle`; only specify `Closeable` if
 * your code requires that the close method is defined.
 *
 * Some types of handle imply these behaviors; for example, all `File\Handle`s
 * are `IO\SeekableHandle`s.
 *
 * You probably want to start with one of:
 * - `File\open_read_only()`, `File\open_write_only()`, or
 *   `File\open_read_write()`
 * - `IO\pipe()`
 * - `IO\request_input()`, `IO\request_input()`, or `IO\request_error()`; these
 *   used for all kinds of requests, including both HTTP and CLI requests.
 * - `IO\server_output()`, `IO\server_error()`
 * - `TCP\connect_async()` or `TCP\Server`
 * - `Unix\connect_async()`, or `Unix\Server`
 */
interface Handle {
}
