<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__DynamicallyCallable, __Memoize>>
function alwaysThrows1() :mixed{
  throw new Exception("Always throws");
}

<<__DynamicallyCallable, __Memoize>>
function alwaysThrows2($a) :mixed{
  throw new Exception("Always throws");
}

class Cls1 {
  <<__DynamicallyCallable, __Memoize>>
  public static function alwaysThrows1() :mixed{
    throw new Exception("Always throws");
  }

  <<__DynamicallyCallable, __Memoize>>
  public static function alwaysThrows2($a) :mixed{
    throw new Exception("Always throws");
  }
}

class Cls2 {
  <<__DynamicallyCallable, __Memoize>>
  public function alwaysThrows1() :mixed{
    throw new Exception("Always throws");
  }

  <<__DynamicallyCallable, __Memoize>>
  public function alwaysThrows2($a) :mixed{
    throw new Exception("Always throws");
  }
}

class Cls3 {
  <<__DynamicallyCallable, __Memoize>>
  public function alwaysThrows() :mixed{
    throw new Exception("Always throws");
  }
}

class Cls4 {
  <<__DynamicallyCallable, __Memoize>>
  public function alwaysThrows($a) :mixed{
    throw new Exception("Always throws");
  }
}

function run($c1, $c2) :mixed{
  try {
    var_dump($c1());
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    var_dump($c1());
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    var_dump($c2(100));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }

  try {
    var_dump($c2(100));
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}

function main() :mixed{
  run(alwaysThrows1<>, alwaysThrows2<>);
  run('Cls1::alwaysThrows1', 'Cls1::alwaysThrows2');
  $a = new Cls2();
  $b = new Cls3();
  $c = new Cls4();
  run(vec[$a, 'alwaysThrows1'], vec[$a, 'alwaysThrows1']);
  run(vec[$b, 'alwaysThrows'], vec[$c, 'alwaysThrows']);
}

<<__EntryPoint>>
function main_unreachable() :mixed{
main();
}
