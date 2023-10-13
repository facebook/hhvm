<?hh
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

final class HackLibTestStringable {
  public function __construct(
    private string $data,
  ) {
  }

  public function __toString(): string {
    return $this->data;
  }
}
