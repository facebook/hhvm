<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__LateInit>> public $p1;
  <<__LateInit>> private $p2;

  <<__LateInit>> static public $s1;
  <<__LateInit>> static private $s2;

  public static function test($a) :mixed{
    var_dump(HH\is_late_init_prop_init($a, 'p2'));
    var_dump(HH\is_late_init_sprop_init('A', 's2'));
  }

  public static function set($a) :mixed{
    $a->p2 = 'abc';
    A::$s2 = 'def';
  }
}

function run_tests($a) :mixed{
  var_dump(HH\is_late_init_prop_init($a, 'p1'));

  try {
    HH\is_late_init_prop_init($a, 'p2');
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    HH\is_late_init_prop_init($a, 'p3');
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  var_dump(HH\is_late_init_sprop_init('A', 's1'));

  try {
    HH\is_late_init_sprop_init('A', 's2');
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    HH\is_late_init_sprop_init('A', 's3');
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  try {
    HH\is_late_init_sprop_init('Foo', 's4');
  } catch (Exception $e) {
    echo $e->getMessage() . "\n";
  }

  A::test($a);
}

<<__EntryPoint>> function test(): void {
  $a = new A();
  run_tests($a);
  $a->p1 = 123;
  A::$s1 = 456;
  A::set($a);
  run_tests($a);
}
