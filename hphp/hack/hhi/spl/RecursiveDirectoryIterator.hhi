<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

class RecursiveDirectoryIterator extends FilesystemIterator
  implements
    RecursiveIterator<SplFileInfo> {

  // Constants
  const FOLLOW_SYMLINKS = 512;

  // Methods
  public function __construct($path, $flags = null);
  public function hasChildren();
  public function getChildren();
  public function getSubPath();
  public function getSubPathname();

}
