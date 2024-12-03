<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// @oss-enable: use namespace FlibSL\{C, Math, Str, Vec};

<<Oncalls('thrift')>> // @oss-disable
final class VecIterator<T> implements Iterator<T> {

  private int $currentIndex = 0;
  private int $count;

  public function __construct(private vec<T> $src)[] {
    $this->count = C\count($src);
  }

  public function current()[]: T {
    invariant($this->valid(), 'iterator not valid');
    return $this->src[$this->currentIndex];
  }

  public function key()[]: int {
    invariant($this->valid(), 'iterator not valid');
    return $this->currentIndex;
  }

  public function next()[write_props]: void {
    $this->currentIndex++;
  }

  public function rewind()[write_props]: void {
    $this->currentIndex = 0;
  }

  public function valid()[]: bool {
    return $this->currentIndex < $this->count;
  }
}
