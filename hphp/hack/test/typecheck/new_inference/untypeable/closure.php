<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Ref<T> {
  private ?T $value = null;
  public function get(T $default): T {
    return $this->value ?? $default;
  }
  public function set(T $value): void {
    $this->value = $value;
  }
}

function getter<T>(Ref<T> $r): (function(T): T) {
  return $x ==> $r->get($x);
}

function expect_string(string $_): void {}

function test(int $a, string $b): void {
  $r = new Ref();
  $f = getter($r);
  $c = $f($b);
  $r->set($a);
  expect_string($c);
}
