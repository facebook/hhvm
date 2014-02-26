<?hh

function flip($input) {
  echo "--------------------\n";
  echo "input: ";
  var_dump($input);
  echo "flip: ";
  var_dump(array_flip($input));
}

function main() {
  flip(['foo', 'bar', 'baz']);
  flip(['a' => 10, 'b' => 20, 'c' => 30]);
  flip(Vector {'foo', 'bar', 'baz'});
  flip(Set {'foo', 'bar', 'baz'});
  flip(Map { 'a' => 10, 'b' => 20, 'c' => 30});
  flip(ImmVector {'foo', 'bar', 'baz'});
  flip(ImmSet {'foo', 'bar', 'baz'});
  flip(ImmMap { 'a' => 10, 'b' => 20, 'c' => 30});

  flip(Vector {new stdClass(), 10, '20', 'str'});

}

main();
