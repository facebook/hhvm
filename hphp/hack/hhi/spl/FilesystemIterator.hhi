<?hh // decl /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

class FilesystemIterator extends DirectoryIterator
  implements
    SeekableIterator<SplFileInfo> {

  // Constants
  const CURRENT_AS_PATHNAME = 32;
  const CURRENT_AS_FILEINFO = 0;
  const CURRENT_AS_SELF = 16;
  const CURRENT_MODE_MASK = 240;
  const KEY_AS_PATHNAME = 0;
  const KEY_AS_FILENAME = 256;
  const FOLLOW_SYMLINKS = 512;
  const KEY_MODE_MASK = 3840;
  const NEW_CURRENT_AND_KEY = 256;
  const SKIP_DOTS = 4096;
  const UNIX_PATHS = 8192;

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
