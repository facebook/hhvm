<?hh // strict
/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the MIT license found in the
 *  LICENSE file in the hphp/hsl/ subdirectory of this source tree.
 *
 */

use namespace HH\Lib\{C, Vec};

/**
 * Iterator that implements the same behavior as generators when
 * Hack.Lang.AutoprimeGenerators is false
 */
final class HackLibTestForwardOnlyIterator<Tk as arraykey, Tv>
implements \HH\Iterator<Tv>, \HH\KeyedIterator<Tk, Tv> {
  private bool $used = false;
  private int $keyIdx = 0;
  private vec<Tk> $keys;

  public function __construct(private dict<Tk, Tv> $data)[] {
    $this->keys = Vec\keys($data);
  }

  public function current()[]: Tv  {
    return $this->data[$this->keys[$this->keyIdx]];
  }

  public function key()[]: Tk {
    return $this->keys[$this->keyIdx];
  }

  public function rewind()[write_props]: void {
    if ($this->used) {
      $this->next();
      $this->used = false;
    }
  }

  public function valid()[]: bool {
    return C\contains_key($this->keys, $this->keyIdx);
  }

  public function next()[write_props]: void {
    $this->used = true;
    $this->keyIdx++;
  }
}
