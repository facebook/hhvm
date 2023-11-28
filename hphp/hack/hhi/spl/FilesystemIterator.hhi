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
  protected HH\FIXME\MISSING_PROP_TYPE $flags;

  // Methods
  public function __construct(
    HH\FIXME\MISSING_PARAM_TYPE $path,
    HH\FIXME\MISSING_PARAM_TYPE $flags = null,
  );
  public function current(): ~SplFileInfo;
  public function getFlags(): HH\FIXME\MISSING_RETURN_TYPE;
  public function key(): HH\FIXME\MISSING_RETURN_TYPE;
  public function next(): void;
  public function rewind(): void;
  public function setFlags(int $flags): HH\FIXME\MISSING_RETURN_TYPE;
  public function seek(int $position): void;
  public function __toString(): string;
}
