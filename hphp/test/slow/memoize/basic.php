<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__Memoize>> function freeFunc1() :mixed{
  echo "freeFunc1()\n";
  return 123;
}
<<__Memoize>> function freeFunc2($a) :mixed{
  echo "freeFunc2()\n";
  return $a;
}
<<__Memoize>> function freeFunc3($a, $b, $c) :mixed{
  echo "freeFunc3()\n";
  return (string)$a . ' ' . (string)$b . ' ' . (string)$c;
}
<<__Memoize>> function freeFunc4() :mixed{
  echo "freeFunc4()\n";
  return null;
}
<<__Memoize>> function freeFunc5($a) :mixed{
  echo "freeFunc5()\n";
  return null;
}

class Cls1 {
  <<__Memoize>>
  public static function staticFunc1() :mixed{
    echo "Cls1::staticFunc1()\n";
    return 123;
  }

  <<__Memoize>>
  public static function staticFunc2($a) :mixed{
    echo "Cls1::staticFunc2()\n";
    return $a;
  }

  <<__Memoize>>
  public static function staticFunc3($a, $b, $c) :mixed{
    echo "Cls1::staticFunc3()\n";
    return (string)$a . ' ' . (string)$b . ' ' . (string)$c;
  }

  <<__Memoize>>
  public static function staticFunc4() :mixed{
    echo "Cls1::staticFunc4()\n";
    return null;
  }

  <<__Memoize>>
  public static function staticFunc5($a) :mixed{
    echo "Cls1::staticFunc5()\n";
    return null;
  }

  <<__Memoize>>
  public function func1() :mixed{
    echo "Cls1::func1()\n";
    return 123;
  }

  <<__Memoize>>
  public function func2($a) :mixed{
    echo "Cls1::func2()\n";
    return $a;
  }

  <<__Memoize>>
  public function func3($a, $b, $c) :mixed{
    echo "Cls1::func3()\n";
    return (string)$a . ' ' . (string)$b . ' ' . (string)$c;
  }

  <<__Memoize>>
  public function func4() :mixed{
    echo "Cls1::func4()\n";
    return null;
  }

  <<__Memoize>>
  public function func5($a) :mixed{
    echo "Cls1::func5()\n";
    return null;
  }
}

class Cls2 {
  <<__Memoize>>
  public function func() :mixed{
    echo "Cls2::func()\n";
    return 123;
  }
}

class Cls3 {
  <<__Memoize>>
  public function func($a) :mixed{
    echo "Cls3::func()\n";
    return $a;
  }
}

class Cls4 {
  <<__Memoize>>
  public function func($a, $b, $c) :mixed{
    echo "Cls4::func()\n";
    return (string)$a . ' ' . (string)$b . ' ' . (string)$c;
  }
}

class Cls5 {
  <<__Memoize>>
  public function func() :mixed{
    echo "Cls5::func()\n";
    return null;
  }
}

class Cls6 {
  <<__Memoize>>
  public function func($a) :mixed{
    echo "Cls6::func()\n";
    return null;
  }
}

trait Trait1 {
  <<__Memoize>>
  public function func1() :mixed{
    echo "Trait1::func1()\n";
    return 123;
  }

  <<__Memoize>>
  public static function staticFunc1() :mixed{
    echo "Trait1::staticFunc1()\n";
    return 123;
  }
}

trait Trait2 {
  <<__Memoize>>
  public function func2($a) :mixed{
    echo "Trait2::func2()\n";
    return $a;
  }

  <<__Memoize>>
  public static function staticFunc2($a) :mixed{
    echo "Trait2::staticFunc2()\n";
    return $a;
  }
}

trait Trait3 {
  <<__Memoize>>
  public function func3($a, $b, $c) :mixed{
    echo "Trait3::func3()\n";
    return (string)$a . ' ' . (string)$b . ' ' . (string)$c;
  }

  <<__Memoize>>
  public static function staticFunc3($a, $b, $c) :mixed{
    echo "Trait3::staticFunc3()\n";
    return (string)$a . ' ' . (string)$b . ' ' . (string)$c;
  }
}

trait Trait4 {
  <<__Memoize>>
  public function func4() :mixed{
    echo "Trait4::func4()\n";
    return null;
  }

  <<__Memoize>>
  public static function staticFunc4() :mixed{
    echo "Trait4::staticFunc4()\n";
    return null;
  }
}

trait Trait5 {
  <<__Memoize>>
  public function func5($a) :mixed{
    echo "Trait5::func5()\n";
    return null;
  }

  <<__Memoize>>
  public static function staticFunc5($a) :mixed{
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

function runFuncs($c1, $c2, $c3, $c4, $c5) :mixed{
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

function runFreeFuncs() :mixed{
  echo "==========================================\n";
  echo "Testing free functions....\n\n";
  runFuncs('freeFunc1',
           'freeFunc2',
           'freeFunc3',
           'freeFunc4',
           'freeFunc5');
}

function runStaticFuncs() :mixed{
  echo "==========================================\n";
  echo "Testing static functions....\n\n";
  runFuncs('Cls1::staticFunc1',
           'Cls1::staticFunc2',
           'Cls1::staticFunc3',
           'Cls1::staticFunc4',
           'Cls1::staticFunc5');
}

function runMethods() :mixed{
  echo "==========================================\n";
  echo "Testing methods....\n\n";
  $a = new Cls1();
  runFuncs(vec[$a, 'func1'],
           vec[$a, 'func2'],
           vec[$a, 'func3'],
           vec[$a, 'func4'],
           vec[$a, 'func5']);

  $b = new Cls1();
  runFuncs(vec[$b, 'func1'],
           vec[$b, 'func2'],
           vec[$b, 'func3'],
           vec[$b, 'func4'],
           vec[$b, 'func5']);
}

function runSingleMethods() :mixed{
  echo "==========================================\n";
  echo "Testing single methods....\n\n";
  $a = new Cls2();
  $b = new Cls3();
  $c = new Cls4();
  $d = new Cls5();
  $e = new Cls6();
  runFuncs(vec[$a, 'func'],
           vec[$b, 'func'],
           vec[$c, 'func'],
           vec[$d, 'func'],
           vec[$e, 'func']);
}

function runTraitStatics() :mixed{
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

function runTraitMethods() :mixed{
  echo "==========================================\n";
  echo "Testing trait methods....\n\n";
  $a = new Cls7();
  runFuncs(vec[$a, 'func1'],
           vec[$a, 'func2'],
           vec[$a, 'func3'],
           vec[$a, 'func4'],
           vec[$a, 'func5']);
  $b = new Cls8();
  runFuncs(vec[$b, 'func1'],
           vec[$b, 'func2'],
           vec[$b, 'func3'],
           vec[$b, 'func4'],
           vec[$b, 'func5']);
}


<<__EntryPoint>>
function main_basic() :mixed{
error_reporting(0);

runFreeFuncs();
runStaticFuncs();
runMethods();
runSingleMethods();
runTraitStatics();
runTraitMethods();
}
