<?hh
/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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
 * @package thrift.protocol.simplephpobject
 */

class TSimplePHPObjectProtocolKeyedIteratorWrapper
  implements Iterator<mixed> {
  private bool $key = true;

  public function __construct(private Iterator<mixed> $itr) {}

  public function key(): string {
    invariant_violation('Cannot Access Key');
  }

  public function current(): mixed {
    if ($this->key) {
      // UNSAFE_BLOCK
      return $this->itr->key();
    } else {
      return $this->itr->current();
    }
  }

  public function next(): void {
    $this->key = !$this->key;
    if ($this->key) {
      $this->itr->next();
    }
  }

  public function rewind(): void {
    $this->key = !$this->key;
    if (!$this->key) {
      $this->itr->rewind();
    }
  }

  public function valid(): bool {
    return $this->itr->valid();
  }
}
