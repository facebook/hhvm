<?hh
/*
 *  Copyright (c) 2017-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the root directory of this source tree.
 *
 */

namespace HH\Lib\IO;

use namespace HH\Lib\OS;

final class BufferedReaderLineIterator implements AsyncIterator<string> {
  public function __construct(private BufferedReader $reader) {
  }

  public async function next(): Awaitable<?(mixed, string)> {
    $line = await $this->reader->readLineAsync();
    if ($line === null) {
      return null;
    }
    return tuple(null, $line);
  }
}
