<?hh

class A {
  public int $x = 12;
}

function attempt(mixed $in): void {
  try {
    var_dump(\HH\serialize_with_options($in, dict['disallowCollections' => true]));
  } catch (Exception $e) {
    echo "threw ". get_class($e).": ".$e->getMessage()."\n";
  }
}


<<__EntryPoint>>
function main(): mixed {
  attempt(new A());
  attempt(dict['a' => 1, 'b' => 2]);
  attempt(vec[1, 2, 3]);
  attempt(keyset[1, 2, 3]);
  attempt(1);
  attempt(1.5);
  attempt("foo");
  attempt(true);
  attempt(null);
  attempt(Map {});
  attempt(Set {});
  attempt(Vector {});
  attempt(Pair {1, 2});
}
