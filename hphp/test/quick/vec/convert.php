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

class AggregateObj implements IteratorAggregate {
  public function getIterator() {
    return new IterableObj();
  }
}

function convert_from($v) {
  echo "====================================================\n";
  var_dump($v);
  echo "----------------------------------------------------\n";
  var_dump(darray($v));
  var_dump(dict(HH\array_unmark_legacy($v)));

  try {
    var_dump(keyset($v));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  var_dump((bool)$v);
  var_dump((int)$v);
  var_dump((float)$v);
  try {
    var_dump((string)$v);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump(new Vector($v));
  var_dump(new Map($v));

  try {
    var_dump(new Set($v));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  echo "====================================================\n";
}

function convert_to($from) {
  echo "====================================================\n";
  var_dump($from);
  echo "----------------------------------------------------\n";
  try {
    var_dump(vec(HH\array_unmark_legacy($from)));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  echo "====================================================\n";
}

<<__EntryPoint>> function main(): void {
  convert_to(varray[]);
  convert_to(varray[100, 'val1', 'val2', 400, null, true, 1.234, new stdClass]);
  convert_to(darray[1 => 10, 'key1' => 'val1', 5 => 'val2', 'key2' => 7,
              10 => null, 15 => true, 'key3' => 1.234,
              'key4' => new stdClass]);

  convert_to(vec[]);
  convert_to(vec[1, 2, 'a', 'b', 3, 4, false, null, 5.67, new stdClass]);

  convert_to(dict[]);
  convert_to(dict[1 => 10, 'key1' => 'val1', 5 => 'val2', 'key2' => 7,
                  10 => null, 15 => true, 'key3' => 1.234,
                  'key4' => new stdClass]);

  convert_to(keyset[]);
  convert_to(keyset[101, 202, 'val1', 'val2', 303]);

  convert_to(false);
  convert_to(null);
  convert_to(123);
  convert_to(1.23);
  convert_to("abcd");
  convert_to(new IterableObj);
  convert_to(new ThrowIterableObj);
  convert_to(new stdClass);
  convert_to(STDIN);
  convert_to(Vector{1, 2, 3});
  convert_to(Map{'a' => 100, 200 => 'b'});
  convert_to(Pair{'a', 'b'});
  convert_to(Set{5, 4, 3, 2, 1});
  convert_to(new AggregateObj);

  convert_from(vec[]);
  convert_from(vec[1, 2, 3, 4]);
  convert_from(vec['z', 'y', 'x']);
  convert_from(vec[1, 'a', 2, 'b']);
  convert_from(vec[vec[1, 2], vec[3, 4, 5]]);
  convert_from(vec[1, 2, false, 'a', 'b']);
  convert_from(vec[1, '1']);
  convert_from(vec['abc', 123, 123, 'abc']);
}
