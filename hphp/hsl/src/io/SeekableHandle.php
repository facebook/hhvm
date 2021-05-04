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

/** A handle that can have its' position changed. */
interface SeekableHandle extends Handle {
  /**
   * Move to a specific offset within a handle.
   *
   * Offset is relative to the start of the handle - so, the beginning of the
   * handle is always offset 0.
   */
  public function seek(int $offset): void;

  /**
   * Get the current pointer position within a handle.
   */
  public function tell(): int;
}
