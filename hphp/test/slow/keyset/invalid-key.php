<?hh

function get($d, $k) {
  try {
    var_dump($d[$k]);
  } catch (InvalidArgumentException $ex) {
    echo "Caught ".$ex->getMessage()."\n";
  }
  return $d;
}

function add($d, $v) {
  try {
    $d[] = $v;
  } catch (InvalidArgumentException $ex) {
    echo "Caught ".$ex->getMessage()."\n";
  }
  return $d;
}

function main() {
  $foo = 12;
  $ref =& $foo;
  $bar = array();
  $badref =& $bar;

  $dyn = 42;
  add(keyset[$dyn], 1)
    |> add($$, "1")
    |> add($$, null)
    |> add($$, array())
    |> add($$, new stdclass)
    |> add($$, 1.2)
    |> add($$, $ref)
    |> add($$, $badref)
    |> add($$, true)
    |> get($$, 1)
    |> get($$, "1")
    |> get($$, null)
    |> get($$, array())
    |> get($$, new stdclass)
    |> get($$, 1.2)
    |> get($$, $ref)
    |> get($$, $badref)
    |> get($$, true)
    |> var_dump($$);
}

main();
