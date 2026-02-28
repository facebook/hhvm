<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

// Don't optimize away "unused" comparisons because they might have
// side-effects.

class Cls1 {
  public function __toString()[] :mixed{
    echo "Cls1::__toString()\n";
    return "FOO";
  }
}

class Cls2 {
  public function __toString() :mixed{
    throw new Exception("Cls2::__toString()");
  }
}

function test_cls1() :mixed{
  $x = new Cls1;
  $s = "hello";

  HH\Lib\Legacy_FIXME\lt($x, $s);
  HH\Lib\Legacy_FIXME\lte($x, $s);
  HH\Lib\Legacy_FIXME\gt($x, $s);
  HH\Lib\Legacy_FIXME\gte($x, $s);
  $x == $s;
  $x != $s;
  HH\Lib\Legacy_FIXME\cmp($x, $s);

  HH\Lib\Legacy_FIXME\lt($s, $x);
  HH\Lib\Legacy_FIXME\lte($s, $x);
  HH\Lib\Legacy_FIXME\gt($s, $x);
  HH\Lib\Legacy_FIXME\gte($s, $x);
  $s == $x;
  $s != $x;
  HH\Lib\Legacy_FIXME\cmp($s, $x);
}

function test_cls2() :mixed{
  $x = new Cls2;
  $s = "hello";

  try { $x < $s; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }
  try { $x <= $s; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }
  try { $x > $s; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }
  try { $x >= $s; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }
  try { $x == $s; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }
  try { $x != $s; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }
  try { $x <=> $s; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }

  try { $s < $x; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }
  try { $s <= $x; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }
  try { $s > $x; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }
  try { $s >= $x; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }
  try { $s == $x; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }
  try { $s != $x; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }
  try { $s <=> $x; } catch (Exception $e) { echo "Exn: " . $e->getMessage() . "\n"; }
}

function test_arr() :mixed{
  $x1 = vec[1, new Cls1(), 3];
  $x2 = vec[1, "bye", 3];

  // <= and >= can evaluate the operands an indeterminate number of times
  // depending on the mode.

  HH\Lib\Legacy_FIXME\lt($x1, $x2);
  //$x1 <= $x2;
  HH\Lib\Legacy_FIXME\gt($x1, $x2);
  //$x1 >= $x2;
  $x1 == $x2;
  $x1 != $x2;
  HH\Lib\Legacy_FIXME\cmp($x1, $x2);

  HH\Lib\Legacy_FIXME\lt($x2, $x1);
  //$x2 <= $x1;
  HH\Lib\Legacy_FIXME\gt($x2, $x1);
  //$x2 >= $x1;
  $x2 == $x1;
  $x2 != $x1;
  HH\Lib\Legacy_FIXME\cmp($x2, $x1);
}


<<__EntryPoint>>
function main_side_effect_compares() :mixed{
test_cls1();
test_cls2();
test_arr();
}
