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

class Str {
  public static function substr($str, $offset, $length=null) {
    $r = is_null($length) ?
      substr($str, $offset) :
      substr($str, $offset, $length);
    if ($r === false) {
      return '';
    }
    return $r;
  }

  public static function startsWith($str, $prefix) {
    return strncmp($str, $prefix, strlen($prefix)) === 0;
  }

  public static function endsWith($str, $suffix) {
    if (strlen($suffix) === 0) {
      return true;
    }
    return strlen($str) >= strlen($suffix) &&
      substr_compare($str, $suffix, -strlen($suffix), strlen($suffix)) === 0;
  }

  public static function pos($haystack, $needle, $offset=0) {
    $i = strpos($haystack, $needle, $offset);
    if ($i === false) {
      return null;
    }
    return $i;
  }
}

function set_obj_memo($obj, $val) {
  ob_start();
  var_export($val);
  $valstr = str_replace(array("\n", "\r"), '', ob_get_clean());
  echo "Setting memo caches in object to $valstr\n";

  $r_obj = new ReflectionObject($obj);
  while ($r_obj instanceof ReflectionClass) {
    $props = $r_obj->getProperties();
    foreach ($r_obj->getProperties() as $prop) {
      if ($prop->isStatic()) { continue; }
      $prop_name = $prop->getName();
      if (Str::startsWith($prop_name, '$shared$') &&
          Str::endsWith($prop_name, '$memoize_cache')) {
        $prop->setAccessible(true);
        $prop->setValue($obj, $val);
      }
    }
    $r_obj = $r_obj->getParentClass();
  }
}

function set_cls_memo($classname, $val) {
  ob_start();
  var_export($val);
  $valstr = str_replace(array("\n", "\r"), '', ob_get_clean());
  echo "Setting memo caches in class \'$classname\' to $valstr\n";

  $r_class = new ReflectionClass($classname);
  foreach ($r_class->getProperties() as $prop) {
    if (!$prop->isStatic()) {
      continue;
    }
    $prop_name = $prop->getName();
    if (!Str::endsWith($prop_name, '$memoize_cache')) {
      continue;
    }
    $method_name = Str::substr($prop_name, 0, Str::pos($prop_name, '$'));
    if (!$r_class->hasMethod($method_name)) {
      continue;
    }
    $prop->setAccessible(true);
    $prop->setValue(null, $val);
  }
}

function runFuncs($c1, $c2) {
  var_dump($c1());
  var_dump($c2(1234));
}

function testStatics($vals) {
  echo "================================\n";
  echo "Testing static functions....\n\n";

  runFuncs('Cls1::func1', 'Cls1::func2');
  runFuncs('Cls1::func1', 'Cls1::func2');
  foreach ($vals as $v) {
    set_cls_memo('Cls1', $v);
    runFuncs('Cls1::func1', 'Cls1::func2');
  }
}

function testMethods($vals) {
  echo "================================\n";
  echo "Testing methods....\n\n";

  $a = new Cls2();
  runFuncs([$a, 'func1'], [$a, 'func2']);
  runFuncs([$a, 'func1'], [$a, 'func2']);
  foreach ($vals as $v) {
    set_obj_memo($a, $v);
    runFuncs([$a, 'func1'], [$a, 'func2']);
  }
}

function testSingleMethods($vals) {
  echo "================================\n";
  echo "Testing single methods....\n\n";

  $a = new Cls3();
  $b = new Cls4();
  runFuncs([$a, 'func'], [$b, 'func']);
  runFuncs([$a, 'func'], [$b, 'func']);
  foreach ($vals as $v) {
    set_obj_memo($a, $v);
    set_obj_memo($b, $v);
    runFuncs([$a, 'func'], [$b, 'func']);
  }
}

function testTraits($vals) {
  echo "================================\n";
  echo "Testing traits....\n\n";

  $a = new Cls5();
  runFuncs([$a, 'func1'], [$a, 'func2']);
  runFuncs([$a, 'func1'], [$a, 'func2']);
  foreach ($vals as $v) {
    set_obj_memo($a, $v);
    runFuncs([$a, 'func1'], [$a, 'func2']);
  }
}

function testTraitStatics($vals) {
  echo "================================\n";
  echo "Testing trait static functions....\n\n";

  runFuncs('Cls6::func1', 'Cls6::func2');
  runFuncs('Cls6::func1', 'Cls6::func2');
  foreach ($vals as $v) {
    set_cls_memo('Cls6', $v);
    runFuncs('Cls6::func1', 'Cls6::func2');
  }
}

function testTraitSingleMethods($vals) {
  echo "================================\n";
  echo "Testing trait single methods....\n\n";

  $a = new Cls7();
  $b = new Cls8();
  runFuncs([$a, 'func'], [$b, 'func']);
  runFuncs([$a, 'func'], [$b, 'func']);
  foreach ($vals as $v) {
    set_obj_memo($a, $v);
    set_obj_memo($b, $v);
    runFuncs([$a, 'func'], [$b, 'func']);
  }
}

function testValid() {
  $vals = [[], vec[], dict[], null];
  testStatics($vals);
  testMethods($vals);
  testSingleMethods($vals);
  testTraits($vals);
  testTraitStatics($vals);
  testTraitSingleMethods($vals);
}

function testInvalidCls($val) { set_cls_memo('Cls1', $val); }
function testInvalidObj1($val) { set_obj_memo(new Cls2(), $val); }
function testInvalidObj2($val) { set_obj_memo(new Cls3(), $val); }
function testInvalidObj3($val) { set_obj_memo(new Cls4(), $val); }
function testInvalidTrait1($val) { set_obj_memo(new Cls5(), $val); }
function testInvalidTrait2($val) { set_cls_memo('Cls6', $val); }
function testInvalidTrait3($val) { set_obj_memo(new Cls7(), $val); }
function testInvalidTrait4($val) { set_obj_memo(new Cls8(), $val); }

function makeWorklist() {
  $invalidVals = [100, true, [1, 2, 3], Vector{},
                  new stdclass(), vec['a', 'b', 'c']];
  $invalidFuncs = ['testInvalidCls', 'testInvalidObj1',
                   'testInvalidObj2', 'testInvalidObj3',
                   'testInvalidTrait1', 'testInvalidTrait2',
                   'testInvalidTrait3', 'testInvalidTrait4'];
  $worklist = [];
  foreach ($invalidFuncs as $f) {
    foreach ($invalidVals as $v) {
      $worklist[] = [$f, $v];
    }
  }
  return $worklist;
}

function main() {
  $count = apc_fetch('count');
  if ($count === false) {
    testValid();
    apc_store('count', 0);
    return;
  }

  $worklist = makeWorklist();
  if ($count >= count($worklist)) return;

  list($func, $val) = $worklist[$count];
  apc_store('count', $count+1);

  $func($val);
}
main();
