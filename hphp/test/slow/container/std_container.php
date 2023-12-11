<?hh

class TestingIterator implements HH\Iterator {
    private $position = 0;
    private $array = vec[
        "second",
        "third",
        "forth"
    ];

    public function __construct() {
        $this->position = 0;
    }

    public function rewind() :mixed{
        $this->position = 0;
    }

    public function current() :mixed{
        return $this->array[$this->position];
    }

    public function key() :mixed{
        return $this->position;
    }

    public function next() :mixed{
        ++$this->position;
    }

    public function valid() :mixed{
        return isset($this->array[$this->position]);
    }
}

class TestingIterable implements HH\Iterable {
    public function getIterator() :mixed{
        return new ArrayIterator(vec["first", "second", "third"]);
    }
}

class TestingIterableInvalid implements HH\Iterable {
    public function getIterator() :mixed{
        return "foo";
    }
}

function test(mixed $x) :mixed{
  print "first: ";
  try {
    var_dump(HH\Lib\_Private\Native\first($x));
  } catch (Exception $e) {
    print $e->getMessage();
    print "\n";
  }

  print "first_key: ";
  try {
    var_dump(HH\Lib\_Private\Native\first_key($x));
  } catch (Exception $e) {
    print $e->getMessage();
    print "\n";
  }

  print "last: ";
  try {
    var_dump(HH\Lib\_Private\Native\last($x));
  } catch (Exception $e) {
    print $e->getMessage();
    print "\n";
  }

  print "last_key: ";
  try {
    var_dump(HH\Lib\_Private\Native\last_key($x));
  } catch (Exception $e) {
    print $e->getMessage();
    print "\n";
  }
}

<<__EntryPoint>>
function main() :mixed{
  print "==========\n";
  print "vec\n";
  print "==========\n";
  test(vec[4, 5, 6]);
  test(vec[4]);
  test(vec[]);

  print "==========\n";
  print "dict\n";
  print "==========\n";
  test(dict["foo" => 5, "bar" => 6, "lo" => 7]);
  test(dict["lo" => 7, "bar" => 6, 0 => 5]);
  test(dict["foo" => 5]);
  test(dict[]);
  $a = dict[];
  $a[0] = 100;
  $a["3"] = 10;
  $a[-1] = 321;
  unset($a[0]);
  test($a);

  print "==========\n";
  print "keyset\n";
  print "==========\n";
  test(keyset[6, 7, 8]);
  test(keyset[8, 7, "6"]);
  test(keyset[6]);
  test(keyset[]);
  $a = keyset[];
  $a[] = 100;
  $a[] = "10";
  $a[] = -1;
  unset($a[-1]);
  test($a);

  print "==========\n";
  print "array\n";
  print "==========\n";
  test(vec[7, 8, 9]);
  test(vec[7]);
  test(vec[]);
  $a = dict[];
  $a[100] = "321";
  $a["0"] = 2;
  $a[-1] = "10";
  test($a);

  print "==========\n";
  print "tuple\n";
  print "==========\n";
  test(tuple(7.9, 8, "nine"));
  test(tuple("nine"));
  test(tuple());

  print "==========\n";
  print "Pair\n";
  print "==========\n";
  test(HH\Pair {"first", "second"});
  test(HH\Pair {"first", 2});

  print "==========\n";
  print "Vector\n";
  print "==========\n";
  test(HH\Vector{4, 5, 6});
  test(HH\Vector{4});
  test(HH\Vector{});

  print "==========\n";
  print "Map\n";
  print "==========\n";
  test(HH\Map{"foo" => 5, "bar" => 6, "lo" => 7});
  test(HH\Map{"lo" => 7, "bar" => 6, "foo" => 5});
  test(HH\Map{"foo" => 5});
  test(HH\Map{});

  print "==========\n";
  print "Set\n";
  print "==========\n";
  test(HH\Set{6, 7, 8});
  test(HH\Set{8, 7, 6});
  test(HH\Set{6});
  test(HH\Set{});

  print "==========\n";
  print "ImmVector\n";
  print "==========\n";
  test(HH\ImmVector{4, 5, 6});
  test(HH\ImmVector{4});
  test(HH\ImmVector{});

  print "==========\n";
  print "ImmMap\n";
  print "==========\n";
  test(HH\ImmMap{"foo" => 5, "bar" => 6, "lo" => 7});
  test(HH\ImmMap{"lo" => 7, "bar" => 6, "foo" => 5});
  test(HH\ImmMap{"foo" => 5});
  test(HH\ImmMap{});

  print "==========\n";
  print "ImmSet\n";
  print "==========\n";
  test(HH\ImmSet{6, 7, 8});
  test(HH\ImmSet{8, 7, 6});
  test(HH\ImmSet{6});
  test(HH\ImmSet{});

  // not supported
  test(new TestingIterable());
  test(new TestingIterator());

  // Failure cases
  test(123);
  test(new TestingIterableInvalid());
}
