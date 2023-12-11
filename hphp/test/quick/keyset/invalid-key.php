<?hh

function get($d, $k) :mixed{
  try {
    var_dump($d[$k]);
  } catch (InvalidArgumentException $ex) {
    echo "Caught ".$ex->getMessage()."\n";
  }
  return $d;
}

function add($d, $v) :mixed{
  try {
    $d[] = $v;
  } catch (InvalidArgumentException $ex) {
    echo "Caught ".$ex->getMessage()."\n";
  }
  return $d;
}

function run(inout $ref, inout $badref) :mixed{
  $dyn = 42;
  add(keyset[$dyn], 1)
    |> add($$, "1")
    |> add($$, null)
    |> add($$, dict[])
    |> add($$, new stdClass)
    |> add($$, 1.2)
    |> add($$, $ref)
    |> add($$, $badref)
    |> add($$, true)
    |> get($$, 1)
    |> get($$, "1")
    |> get($$, null)
    |> get($$, dict[])
    |> get($$, new stdClass)
    |> get($$, 1.2)
    |> get($$, $ref)
    |> get($$, $badref)
    |> get($$, true)
    |> var_dump($$);
}

<<__EntryPoint>>
function main() :mixed{
  $foo = 12;
  $bar = dict[];
  run(inout $foo, inout $bar);
}
