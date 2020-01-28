<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__Memoize>>
function alwaysThrows1() {
  throw new Exception("Always throws");
}

<<__Memoize>>
function alwaysThrows2($a) {
  throw new Exception("Always throws");
}

class Cls1 {
  <<__Memoize>>
  public static function alwaysThrows1() {
    throw new Exception("Always throws");
  }

  <<__Memoize>>
  public static function alwaysThrows2($a) {
    throw new Exception("Always throws");
  }
}

class Cls2 {
  <<__Memoize>>
  public function alwaysThrows1() {
    throw new Exception("Always throws");
  }

  <<__Memoize>>
  public function alwaysThrows2($a) {
    throw new Exception("Always throws");
  }
}

class Cls3 {
  <<__Memoize>>
  public function alwaysThrows() {
    throw new Exception("Always throws");
  }
}

class Cls4 {
  <<__Memoize>>
  public function alwaysThrows($a) {
    throw new Exception("Always throws");
  }
}

function run($c1, $c2) {
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

function main() {
  run('alwaysThrows1', 'alwaysThrows2');
  run('Cls1::alwaysThrows1', 'Cls1::alwaysThrows2');
  $a = new Cls2();
  $b = new Cls3();
  $c = new Cls4();
  run(varray[$a, 'alwaysThrows1'], varray[$a, 'alwaysThrows1']);
  run(varray[$b, 'alwaysThrows'], varray[$c, 'alwaysThrows']);
}

<<__EntryPoint>>
function main_unreachable() {
main();
}
