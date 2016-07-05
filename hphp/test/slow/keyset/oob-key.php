<?hh

function get($d, $k) {
  try {
    var_dump($d[$k]);
  } catch (OutOfBoundsException $ex) {
    echo "Caught ".$ex->getMessage()."\n";
  }
  return $d;
}

function manipulate($arr) {
  $arr['foo'][] = 12;
  var_dump($arr['foo'][1]);
  try {
    var_dump($arr['foo'][2]);
  } catch (OutOfBoundsException $ex) {
    echo "Caught ".$ex->getMessage()."\n";
  }
  var_dump($arr);
}

function main() {
  function init($b) {
    if ($b) { return 'a'; }
    return 'b';
  }
  $foo = keyset[init(true), init(false)];
  $foo[] = 1;
  $foo[] = '2';
  $foo[] = 3;
  $foo[] = '4';

  get($foo, 1)
    |> get($$, '2')
    |> get($$, 3)
    |> get($$, '4')
    |> get($$, 'a')
    |> get($$, 'c')
    |> get($$, '1')
    |> get($$, 2)
    |> get($$, '3')
    |> get($$, 4)
    |> get($$, 'b')
    |> get($$, 'd')
    |> get($$, 'foobar')
    |> var_dump($$);

  $arr = array('foo' => keyset[$foo[1], '2', 3]);
  manipulate($arr);
}

main();
