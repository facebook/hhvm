<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class FilesystemIterator extends DirectoryIterator {

  // Constants
  const int CURRENT_AS_PATHNAME;
  const int CURRENT_AS_FILEINFO;
  const int CURRENT_AS_SELF;
  const int CURRENT_MODE_MASK;
  const int KEY_AS_PATHNAME;
  const int KEY_AS_FILENAME;
  const int FOLLOW_SYMLINKS;
  const int KEY_MODE_MASK;
  const int NEW_CURRENT_AND_KEY;
  const int SKIP_DOTS;
  const int UNIX_PATHS;

  // Properties
  protected $flags;

  // Methods
  public function __construct($path, $flags = null);
  public function current(): HH\FIXME\POISON_MARKER<SplFileInfo>;
  public function getFlags();
  public function key();
  public function next(): void;
  public function rewind(): void;
  public function setFlags(int $flags);
  public function seek(int $position): void;
  public function __toString(): string;
}
