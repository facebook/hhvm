<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

// Don't optimize away "unused" comparisons because they might have
// side-effects.

class Cls1 {
  public function __toString() {
    echo "Cls1::__toString()\n";
    return "FOO";
  }
}

class Cls2 {
  public function __toString() {
    throw new Exception("Cls2::__toString()");
  }
}

function test_cls1() {
  $x = new Cls1;
  $s = "hello";

  $x < $s;
  $x <= $s;
  $x > $s;
  $x >= $s;
  $x == $s;
  $x != $s;
  $x <=> $s;

  $s < $x;
  $s <= $x;
  $s > $x;
  $s >= $x;
  $s == $x;
  $s != $x;
  $s <=> $x;
}

function test_cls2() {
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

function test_arr() {
  $x1 = varray[1, new Cls1(), 3];
  $x2 = varray[1, "bye", 3];

  // <= and >= can evaluate the operands an indeterminate number of times
  // depending on the mode.

  $x1 < $x2;
  //$x1 <= $x2;
  $x1 > $x2;
  //$x1 >= $x2;
  $x1 == $x2;
  $x1 != $x2;
  $x1 <=> $x2;

  $x2 < $x1;
  //$x2 <= $x1;
  $x2 > $x1;
  //$x2 >= $x1;
  $x2 == $x1;
  $x2 != $x1;
  $x2 <=> $x1;
}


<<__EntryPoint>>
function main_side_effect_compares() {
test_cls1();
test_cls2();
test_arr();
}
