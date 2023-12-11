<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class IterableObj implements Iterator {
  private int $position = 0;
  public function __construct() { $this->position = 0; }
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
  public function __construct() { $this->position = 0; }
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
  public function getIterator() :mixed{
    return new IterableObj();
  }
}

function convert_from($d) :mixed{
  echo "====================================================\n";
  var_dump($d);
  echo "----------------------------------------------------\n";
  var_dump(darray($d));
  var_dump(vec(HH\array_unmark_legacy($d)));

  try {
    var_dump(keyset($d));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  var_dump((bool)$d);
  var_dump((int)$d);
  var_dump((float)$d);
  try {
    var_dump((string)$d);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  var_dump(new Vector($d));
  var_dump(new Map($d));

  try {
    var_dump(new Set($d));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }

  echo "====================================================\n";
}

function convert_to($from) :mixed{
  echo "====================================================\n";
  var_dump($from);
  echo "----------------------------------------------------\n";
  try {
    var_dump(dict(HH\array_unmark_legacy($from)));
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  echo "====================================================\n";
}

<<__EntryPoint>> function main(): void {
  convert_to(vec[]);
  convert_to(vec[100, 'val1', 'val2', 400, null, true, 1.234, new stdClass]);
  convert_to(dict[1 => 10, 'key1' => 'val1', 5 => 'val2', 'key2' => 7,
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
  convert_to(keyset[1, '1']);

  convert_to(false);
  convert_to(null);
  convert_to(123);
  convert_to(1.23);
  convert_to("abcd");
  convert_to(new IterableObj);
  convert_to(new ThrowIterableObj);
  convert_to(new stdClass);
  convert_to(HH\stdin());
  convert_to(Vector{1, 2, 3});
  convert_to(Map{'a' => 100, 200 => 'b'});
  convert_to(Pair{'a', 'b'});
  convert_to(Set{5, 4, 3, 2, 1});
  convert_to(new AggregateObj);

  convert_from(dict[]);
  convert_from(dict[1 => 1, 2 => true, 3 => null, 4 => vec[]]);
  convert_from(dict['key1' => 'val1', 'key2' => 'val2', 'key3' => 'val3']);
  convert_from(dict['key1' => 100, 'key2' => 'val1',
                    200 => 300, 400 => 'val2']);
  convert_from(dict['12345' => 100, 12345 => 200]);
  convert_from(dict[1 => dict[5 => 10], 'a' => dict['b' => 'c']]);
}
