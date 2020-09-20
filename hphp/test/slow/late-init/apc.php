<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public $x = 123;
  <<__LateInit>> public $y;
  public $z = 'abc';
}

<<__EntryPoint>> function test(): void {
  $a = new A();
  try {
    echo "============== store #1 =================\n";
    apc_store('a-key1', $a);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  $a->y = 700;
  try {
    echo "============== store #2 =================\n";
    apc_store('a-key2', $a);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  unset($a->y);
  try {
    echo "============== store #3 =================\n";
    apc_store('a-key3', $a);
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

}
