<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

// LateInit props, even when unset, don't trigger magic getter

class A {
  <<__LateInit>> private $x;
  public function __get($name) { return 123; }
  public function test() { unset($this->x); return $this->x; }
}
<<__EntryPoint>> function main(): void {
$a = new A();
try {
  var_dump($a->test());
} catch (Exception $e) {
  echo $e->getMessage() . "\n";
}
}
