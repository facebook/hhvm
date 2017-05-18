<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class IterableObj implements Iterator {
  private int $position = 0;
  public function __construct() { $this->position = 0; }
  public function rewind() { $this->position = 0; }
  public function current() {
    if ($this->position == 0) return "abc";
    if ($this->position == 1) return "def";
    if ($this->position == 2) return "ghi";
  }
  public function key() {
    if ($this->position == 0) return 100;
    if ($this->position == 1) return 200;
    if ($this->position == 2) return 300;
  }
  public function next() { ++$this->position; }
  public function valid() { return $this->position < 3; }
}

class ThrowIterableObj implements Iterator {
  private int $position = 0;
  public function __construct() { $this->position = 0; }
  public function rewind() { $this->position = 0; }
  public function current() {
    if ($this->position == 0) return "abc";
    if ($this->position == 1) return "def";
    if ($this->position == 2) return "ghi";
  }
  public function key() {
    if ($this->position == 0) return 100;
    if ($this->position == 1) return 200;
    if ($this->position == 2) return 300;
  }
  public function next() {
    ++$this->position;
    if ($this->position == 2) throw new Exception("ThrowIterableObj");
  }
  public function valid() { return $this->position < 3; }
}

function test_varray($v) {
  echo "============== test_varray =========================\n";
  try {
    var_dump(varray($v));
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

function test_darray($v) {
  echo "============== test_darray =========================\n";
  try {
    var_dump(darray($v));
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}
function test_indirect($c, $v) {
  echo "============== test_indirect ($c) ==================\n";
  try {
    var_dump($c($v));
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

$values = vec[
  null,
  false,
  true,
  'abc',
  123,
  '123',
  3.14,
  [],
  [1, 2, 3, 4],
  ['a' => 100, 'b' => 200, 'c' => 300],
  vec[],
  vec[1, 2, 3, 4],
  dict[],
  dict[1 => 'a', 2 => 'b'],
  keyset[],
  keyset[100, 'abc', 200],
  STDIN,
  new stdclass,
  new IterableObj,
  new ThrowIterableObj,
  dict[100 => 'abc', '100' => 'def'],
  keyset[100, '100'],
  Vector{100, 200, 300},
  Set{'a', 'b', 'c', 'd'},
  Map{100 => 'a', 200 => 'b', 300 => 'c'},
  Pair{'a', 100}
];
$values = __hhvm_intrinsics\launder_value($values);

foreach ($values as $v) {
  test_varray($v);
}
foreach ($values as $v) {
  test_darray($v);
}

$c1 = __hhvm_intrinsics\launder_value('HH\\varray');
$c2 = __hhvm_intrinsics\launder_value('HH\\darray');
foreach ($values as $v) {
  test_indirect('HH\\varray', $v);
}
foreach ($values as $v) {
  test_indirect('HH\\darray', $v);
}
