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

function run(inout $ref, inout $badref) {
  $dyn = 42;
  add(keyset[$dyn], 1)
    |> add($$, "1")
    |> add($$, null)
    |> add($$, darray[])
    |> add($$, new stdclass)
    |> add($$, 1.2)
    |> add($$, $ref)
    |> add($$, $badref)
    |> add($$, true)
    |> get($$, 1)
    |> get($$, "1")
    |> get($$, null)
    |> get($$, darray[])
    |> get($$, new stdclass)
    |> get($$, 1.2)
    |> get($$, $ref)
    |> get($$, $badref)
    |> get($$, true)
    |> var_dump($$);
}

<<__EntryPoint>>
function main() {
  $foo = 12;
  $bar = darray[];
  run(inout $foo, inout $bar);
}
