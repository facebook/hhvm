<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__Memoize>> function freeFunc1() {
  echo "freeFunc1()\n";
  return 123;
}
<<__Memoize>> function freeFunc2($a) {
  echo "freeFunc2()\n";
  return $a;
}
<<__Memoize>> function freeFunc3($a, $b, $c) {
  echo "freeFunc3()\n";
  return (string)$a . ' ' . (string)$b . ' ' . (string)$c;
}
<<__Memoize>> function freeFunc4() {
  echo "freeFunc4()\n";
  return null;
}
<<__Memoize>> function freeFunc5($a) {
  echo "freeFunc5()\n";
  return null;
}

class Cls1 {
  <<__Memoize>>
  public static function staticFunc1() {
    echo "Cls1::staticFunc1()\n";
    return 123;
  }

  <<__Memoize>>
  public static function staticFunc2($a) {
    echo "Cls1::staticFunc2()\n";
    return $a;
  }

  <<__Memoize>>
  public static function staticFunc3($a, $b, $c) {
    echo "Cls1::staticFunc3()\n";
    return (string)$a . ' ' . (string)$b . ' ' . (string)$c;
  }

  <<__Memoize>>
  public static function staticFunc4() {
    echo "Cls1::staticFunc4()\n";
    return null;
  }

  <<__Memoize>>
  public static function staticFunc5($a) {
    echo "Cls1::staticFunc5()\n";
    return null;
  }

  <<__Memoize>>
  public function func1() {
    echo "Cls1::func1()\n";
    return 123;
  }

  <<__Memoize>>
  public function func2($a) {
    echo "Cls1::func2()\n";
    return $a;
  }

  <<__Memoize>>
  public function func3($a, $b, $c) {
    echo "Cls1::func3()\n";
    return (string)$a . ' ' . (string)$b . ' ' . (string)$c;
  }

  <<__Memoize>>
  public function func4() {
    echo "Cls1::func4()\n";
    return null;
  }

  <<__Memoize>>
  public function func5($a) {
    echo "Cls1::func5()\n";
    return null;
  }
}

class Cls2 {
  <<__Memoize>>
  public function func() {
    echo "Cls2::func()\n";
    return 123;
  }
}

class Cls3 {
  <<__Memoize>>
  public function func($a) {
    echo "Cls3::func()\n";
    return $a;
  }
}

class Cls4 {
  <<__Memoize>>
  public function func($a, $b, $c) {
    echo "Cls4::func()\n";
    return (string)$a . ' ' . (string)$b . ' ' . (string)$c;
  }
}

class Cls5 {
  <<__Memoize>>
  public function func() {
    echo "Cls5::func()\n";
    return null;
  }
}

class Cls6 {
  <<__Memoize>>
  public function func($a) {
    echo "Cls6::func()\n";
    return null;
  }
}

trait Trait1 {
  <<__Memoize>>
  public function func1() {
    echo "Trait1::func1()\n";
    return 123;
  }

  <<__Memoize>>
  public static function staticFunc1() {
    echo "Trait1::staticFunc1()\n";
    return 123;
  }
}

trait Trait2 {
  <<__Memoize>>
  public function func2($a) {
    echo "Trait2::func2()\n";
    return $a;
  }

  <<__Memoize>>
  public static function staticFunc2($a) {
    echo "Trait2::staticFunc2()\n";
    return $a;
  }
}

trait Trait3 {
  <<__Memoize>>
  public function func3($a, $b, $c) {
    echo "Trait3::func3()\n";
    return (string)$a . ' ' . (string)$b . ' ' . (string)$c;
  }

  <<__Memoize>>
  public static function staticFunc3($a, $b, $c) {
    echo "Trait3::staticFunc3()\n";
    return (string)$a . ' ' . (string)$b . ' ' . (string)$c;
  }
}

trait Trait4 {
  <<__Memoize>>
  public function func4() {
    echo "Trait4::func4()\n";
    return null;
  }

  <<__Memoize>>
  public static function staticFunc4() {
    echo "Trait4::staticFunc4()\n";
    return null;
  }
}

trait Trait5 {
  <<__Memoize>>
  public function func5($a) {
    echo "Trait5::func5()\n";
    return null;
  }

  <<__Memoize>>
  public static function staticFunc5($a) {
    echo "Trait5::staticFunc5()\n";
    return null;
  }
}

class Cls7 {
  use Trait1;
  use Trait2;
  use Trait3;
  use Trait4;
  use Trait5;
}

class Cls8 {
  use Trait1;
  use Trait2;
  use Trait3;
  use Trait4;
  use Trait5;
}

function runFuncs($c1, $c2, $c3, $c4, $c5) {
  var_dump($c1());
  var_dump($c1());
  var_dump($c1('abc'));
  var_dump($c1('abc'));

  var_dump($c2('abc'));
  var_dump($c2(456));
  var_dump($c2('abc'));
  var_dump($c2(456));
  try { var_dump($c2()); } catch (Exception $e) { var_dump($e->getMessage()); }
  var_dump($c2(null));
  var_dump($c2(456, 'abc'));
  var_dump($c2('abc', 456));

  var_dump($c3(789, 'def', true));
  var_dump($c3(false, 100, 1.23));
  var_dump($c3(789, 'def', false));
  var_dump($c3(789, 'def', true));
  var_dump($c3(false, 100, 1.23));
  var_dump($c3(789, 'def', false));
  try { var_dump($c3(123, 'abc')); } catch (Exception $e) { var_dump($e->getMessage()); }
  var_dump($c3(123, 'abc', null));
  var_dump($c3(456, 123, 'ghi', 789));
  var_dump($c3(456, 123, 'ghi', 987));

  var_dump($c4());
  var_dump($c4());
  var_dump($c4('abc'));
  var_dump($c4('abc'));

  var_dump($c5(123));
  var_dump($c5('abc'));
  var_dump($c5(123));
  var_dump($c5('abc'));
  try { var_dump($c5()); } catch (Exception $e) { var_dump($e->getMessage()); }
  var_dump($c5(null));
  var_dump($c5(123, 'abc'));
  var_dump($c5('abc', 123));
}

function runFreeFuncs() {
  echo "==========================================\n";
  echo "Testing free functions....\n\n";
  runFuncs('freeFunc1',
           'freeFunc2',
           'freeFunc3',
           'freeFunc4',
           'freeFunc5');
}

function runStaticFuncs() {
  echo "==========================================\n";
  echo "Testing static functions....\n\n";
  runFuncs('Cls1::staticFunc1',
           'Cls1::staticFunc2',
           'Cls1::staticFunc3',
           'Cls1::staticFunc4',
           'Cls1::staticFunc5');
}

function runMethods() {
  echo "==========================================\n";
  echo "Testing methods....\n\n";
  $a = new Cls1();
  runFuncs(varray[$a, 'func1'],
           varray[$a, 'func2'],
           varray[$a, 'func3'],
           varray[$a, 'func4'],
           varray[$a, 'func5']);

  $b = new Cls1();
  runFuncs(varray[$b, 'func1'],
           varray[$b, 'func2'],
           varray[$b, 'func3'],
           varray[$b, 'func4'],
           varray[$b, 'func5']);
}

function runSingleMethods() {
  echo "==========================================\n";
  echo "Testing single methods....\n\n";
  $a = new Cls2();
  $b = new Cls3();
  $c = new Cls4();
  $d = new Cls5();
  $e = new Cls6();
  runFuncs(varray[$a, 'func'],
           varray[$b, 'func'],
           varray[$c, 'func'],
           varray[$d, 'func'],
           varray[$e, 'func']);
}

function runTraitStatics() {
  echo "==========================================\n";
  echo "Testing trait statics....\n\n";
  runFuncs('Cls7::staticFunc1',
           'Cls7::staticFunc2',
           'Cls7::staticFunc3',
           'Cls7::staticFunc4',
           'Cls7::staticFunc5');
  runFuncs('Cls8::staticFunc1',
           'Cls8::staticFunc2',
           'Cls8::staticFunc3',
           'Cls8::staticFunc4',
           'Cls8::staticFunc5');
}

function runTraitMethods() {
  echo "==========================================\n";
  echo "Testing trait methods....\n\n";
  $a = new Cls7();
  runFuncs(varray[$a, 'func1'],
           varray[$a, 'func2'],
           varray[$a, 'func3'],
           varray[$a, 'func4'],
           varray[$a, 'func5']);
  $b = new Cls8();
  runFuncs(varray[$b, 'func1'],
           varray[$b, 'func2'],
           varray[$b, 'func3'],
           varray[$b, 'func4'],
           varray[$b, 'func5']);
}


<<__EntryPoint>>
function main_basic() {
error_reporting(0);

runFreeFuncs();
runStaticFuncs();
runMethods();
runSingleMethods();
runTraitStatics();
runTraitMethods();
}
