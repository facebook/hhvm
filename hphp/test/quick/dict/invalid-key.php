<?hh

function get(dict $d, mixed $k): dict {
  try {
    var_dump($d[$k]);
  } catch (InvalidArgumentException $ex) {
    echo "Caught ".$ex->getMessage()."\n";
  }
  return $d;
}

function set(dict $d, mixed $k, mixed $v): dict {
  try {
    $d[$k] = $v;
  } catch (InvalidArgumentException $ex) {
    echo "Caught ".$ex->getMessage()."\n";
  }
  return $d;
}

function run(inout $ref, inout $badref) {
  set(dict[], "1", "hello")
    |> set($$, 1, "goodbye")
    |> set($$, null, "null")
    |> set($$, darray[], "arr")
    |> set($$, new stdclass, "cls")
    |> set($$, 1.2, "double")
    |> set($$, $ref, "num-ref")
    |> set($$, $badref, "arr-ref")
    |> set($$, true, "bool")
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
