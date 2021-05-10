<?hh // strict

/**
 * Copyright (c), Facebook, Inc and affiliates
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */
class AAA {
  protected function foo(): string { return 'a'; }
}

class BBB extends AAA {
  protected function foo(): string { return 'b'; }
}

class CCC extends AAA {
  public function baz(): string { return $this->bbb()->foo(); }
  private function bbb(): AAA { return new BBB(); }
}

<<__EntryPoint>>
function main(): void {
  $c = new CCC();
  // returns 'b'
  $x = $c->baz();
  echo "$x\n";
}
