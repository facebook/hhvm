<?hh
/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

final class C<+T> {
  private ?Box<this> $mybox = null;

  public function __construct(private T $data) {
    $this->mybox = new Box($this);
  }

  public function getBox(): Box<this> {
    return $this->mybox as nonnull;
  }

  public function get(): T {
    return $this->data;
  }
}

class Box<T> {
  public function __construct(public T $data) {}
}

function test(C<arraykey> $x): void {
  $y = $x->getBox();
  $y->data = new C(42);
}

function testx(): string {
  $x = new C<string>("hi");
  test($x); // we just stored a int (42) in a string location
  return $x->getBox()->data->get();
}

<<__EntryPoint>>
function main(): void {
  testx();
}
