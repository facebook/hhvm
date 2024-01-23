<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class IterableObj implements Iterator {
  private int $position = 0;
  public function __construct()[] { $this->position = 0; }
  public function rewind() :mixed{ $this->position = 0; }
  public function current() :mixed{
    if ($this->position == 0) return "abc";
    if ($this->position == 1) return "def";
    if ($this->position == 2) return "ghi";
  }
  public function key() :mixed{
    if ($this->position == 0) return 100;
    if ($this->position == 1) return 200;
    if ($this->position == 2) return 300;
  }
  public function next() :mixed{ ++$this->position; }
  public function valid() :mixed{ return $this->position < 3; }
}

class ThrowIterableObj implements Iterator {
  private int $position = 0;
  public function __construct()[] { $this->position = 0; }
  public function rewind() :mixed{ $this->position = 0; }
  public function current() :mixed{
    if ($this->position == 0) return "abc";
    if ($this->position == 1) return "def";
    if ($this->position == 2) return "ghi";
  }
  public function key() :mixed{
    if ($this->position == 0) return 100;
    if ($this->position == 1) return 200;
    if ($this->position == 2) return 300;
  }
  public function next() :mixed{
    ++$this->position;
    if ($this->position == 2) throw new Exception("ThrowIterableObj");
  }
  public function valid() :mixed{ return $this->position < 3; }
}

class AggregateObj implements IteratorAggregate {
  public function getIterator()[] :mixed{
    return new IterableObj();
  }
}

function test_varray($v) :mixed{
  echo "============== test_varray =========================\n";
  try {
    $v2 = varray($v);
    var_dump($v2);
    var_dump(is_varray($v2));
    var_dump(is_darray($v2));
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

function test_darray($v) :mixed{
  echo "============== test_darray =========================\n";
  try {
    $v2 = darray($v);
    var_dump($v2);
    var_dump(is_varray($v2));
    var_dump(is_darray($v2));
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}

function test_indirect($c, $v) :mixed{
  echo "============== test_indirect ($c) ==================\n";
  try {
    $v2 = $c($v);
    var_dump($v2);
    var_dump(is_varray($v2));
    var_dump(is_darray($v2));
  } catch (Exception $e) {
    echo "Exception: " . $e->getMessage() . "\n";
  }
}


<<__EntryPoint>>
function main_convert(): void {
  $values = vec[
    null,
    false,
    true,
    'abc',
    123,
    '123',
    3.14,
    dict[],
    darray(vec[1, 2, 3, 4]),
    darray(dict['a' => 100, 'b' => 200, 'c' => 300]),
    vec[],
    vec[1, 2, 3, 4],
    dict[],
    dict[1 => 'a', 2 => 'b'],
    keyset[],
    keyset[100, 'abc', 200],
    HH\stdin(),
    new stdClass,
    new IterableObj,
    new ThrowIterableObj,
    new AggregateObj,
    dict[100 => 'abc', '100' => 'def'],
    keyset[100, '100'],
    Vector{100, 200, 300},
    Set{'a', 'b', 'c', 'd'},
    Map{100 => 'a', 200 => 'b', 300 => 'c'},
    Pair{'a', 100},
    vec[],
    vec['a', 'b', 'c'],
    dict[],
    dict[0 => 'x', 1 => 'y', 2 => 'z'],
    dict['key1' => 111, 'key2' => 222]
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
    test_indirect($c1, $v);
  }
  foreach ($values as $v) {
    test_indirect($c2, $v);
  }
}
