<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Cls1 {
  <<__Memoize>>
  public static function func1() {
    echo "Cls1::func1()\n";
    return 'abcdef';
  }

  <<__Memoize>>
  public static function func2($a) {
    echo "Cls1::func2()\n";
    return $a;
  }
}

class Cls2 {
  <<__Memoize>>
  public function func1() {
    echo "Cls2::func1()\n";
    return 'ghijkl';
  }

  <<__Memoize>>
  public function func2($a) {
    echo "Cls2::func2()\n";
    return $a;
  }
}

class Cls3 {
  <<__Memoize>>
  public function func() {
    echo "Cls3::func()\n";
    return 'xyz';
  }
}

class Cls4 {
  <<__Memoize>>
  public function func($a) {
    echo "Cls4::func()\n";
    return $a;
  }
}

trait Trait1 {
  <<__Memoize>>
  public function func1() {
    echo "Trait1::func1()\n";
    return 123;
  }

  <<__Memoize>>
  public function func2($a) {
    echo "Trait1::func2()\n";
    return $a;
  }
}

trait Trait2 {
  <<__Memoize>>
  public static function func1() {
    echo "Trait2::func1()\n";
    return 'qwerty';
  }

  <<__Memoize>>
  public static function func2($a) {
    echo "Trait2::func2()\n";
    return $a;
  }
}

trait Trait3 {
  <<__Memoize>>
  public function func() {
    echo "Trait3::func()\n";
    return 'xyz';
  }
}

trait Trait4 {
  <<__Memoize>>
  public function func($a) {
    echo "Trait4::func()\n";
    return $a;
  }
}

class Cls5 {
  use Trait1;
}

class Cls6 {
  use Trait2;
}

class Cls7 {
  use Trait3;
}

class Cls8 {
  use Trait4;
}

<<__Memoize>> function func1() {
  echo "func1()\n";
  return 'zyzzy';
}

<<__Memoize>> function func2($a) {
  echo "func2()\n";
  return $a;
}

function runFuncs($c1, $c2) {
  var_dump($c1());
  var_dump($c2(1234));
}

function testStatics() {
  echo "================================\n";
  echo "Testing static functions....\n\n";
  runFuncs('Cls1::func1', 'Cls1::func2');
  runFuncs('Cls1::func1', 'Cls1::func2');
  HH\clear_static_memoization('Cls1', 'func1');
  runFuncs('Cls1::func1', 'Cls1::func2');
  HH\clear_static_memoization('Cls1', 'func2');
  runFuncs('Cls1::func1', 'Cls1::func2');
  HH\clear_static_memoization('Cls1');
  runFuncs('Cls1::func1', 'Cls1::func2');
}

function testMethods() {
  echo "================================\n";
  echo "Testing methods....\n\n";
  $a = new Cls2();
  runFuncs(varray[$a, 'func1'], varray[$a, 'func2']);
  runFuncs(varray[$a, 'func1'], varray[$a, 'func2']);
  HH\clear_instance_memoization($a);
  runFuncs(varray[$a, 'func1'], varray[$a, 'func2']);
}

function testSingleMethods() {
  echo "================================\n";
  echo "Testing single methods....\n\n";
  $a = new Cls3();
  $b = new Cls4();
  runFuncs(varray[$a, 'func'], varray[$b, 'func']);
  runFuncs(varray[$a, 'func'], varray[$b, 'func']);
  HH\clear_instance_memoization($a);
  runFuncs(varray[$a, 'func'], varray[$b, 'func']);
  HH\clear_instance_memoization($b);
  runFuncs(varray[$a, 'func'], varray[$b, 'func']);
}

function testTraits() {
  echo "================================\n";
  echo "Testing traits....\n\n";
  $a = new Cls5();
  runFuncs(varray[$a, 'func1'], varray[$a, 'func2']);
  runFuncs(varray[$a, 'func1'], varray[$a, 'func2']);
  HH\clear_instance_memoization($a);
  runFuncs(varray[$a, 'func1'], varray[$a, 'func2']);
}

function testTraitStatics() {
  echo "================================\n";
  echo "Testing trait static functions....\n\n";
  runFuncs('Cls6::func1', 'Cls6::func2');
  runFuncs('Cls6::func1', 'Cls6::func2');
  HH\clear_static_memoization('Cls6', 'func1');
  runFuncs('Cls6::func1', 'Cls6::func2');
  HH\clear_static_memoization('Cls6', 'func2');
  runFuncs('Cls6::func1', 'Cls6::func2');
  HH\clear_static_memoization('Cls6');
  runFuncs('Cls6::func1', 'Cls6::func2');
}

function testTraitSingleMethods() {
  echo "================================\n";
  echo "Testing trait single methods....\n\n";
  $a = new Cls7();
  $b = new Cls8();
  runFuncs(varray[$a, 'func'], varray[$b, 'func']);
  runFuncs(varray[$a, 'func'], varray[$b, 'func']);
  HH\clear_instance_memoization($a);
  runFuncs(varray[$a, 'func'], varray[$b, 'func']);
  HH\clear_instance_memoization($b);
  runFuncs(varray[$a, 'func'], varray[$b, 'func']);
}

function testFreeFuncs() {
  echo "================================\n";
  echo "Testing free functions....\n\n";
  runFuncs('func1', 'func2');
  runFuncs('func1', 'func2');
  HH\clear_static_memoization(null, 'func1');
  runFuncs('func1', 'func2');
  HH\clear_static_memoization(null, 'func2');
  runFuncs('func1', 'func2');
}

function main() {
  testStatics();
  testMethods();
  testSingleMethods();
  testTraits();
  testTraitStatics();
  testTraitSingleMethods();
  testFreeFuncs();
}

<<__EntryPoint>>
function main_reset() {
main();
}
