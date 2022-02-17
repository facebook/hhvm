<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class FilesystemIterator extends DirectoryIterator
  implements
    SeekableIterator<SplFileInfo> {

  // Constants
  const int CURRENT_AS_PATHNAME = 32;
  const int CURRENT_AS_FILEINFO = 0;
  const int CURRENT_AS_SELF = 16;
  const int CURRENT_MODE_MASK = 240;
  const int KEY_AS_PATHNAME = 0;
  const int KEY_AS_FILENAME = 256;
  const int FOLLOW_SYMLINKS = 512;
  const int KEY_MODE_MASK = 3840;
  const int NEW_CURRENT_AND_KEY = 256;
  const int SKIP_DOTS = 4096;
  const int UNIX_PATHS = 8192;

  // Properties
  protected $flags;

  // Methods
  public function __construct($path, $flags = null);
  public function current();
  public function getFlags();
  public function key();
  public function next();
  public function rewind();
  public function setFlags(int $flags);
  public function seek(int $position): void;
  public function __toString();
}
