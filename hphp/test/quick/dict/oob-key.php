<?hh

function get(dict $d, mixed $k): dict {
  try {
    var_dump($d[$k]);
  } catch (OutOfBoundsException $ex) {
    echo "Caught ".$ex->getMessage()."\n";
  }
  return $d;
}

function manipulate($arr) :mixed{
  try {
    $arr['foo']['bar'][] = 12;
  } catch (OutOfBoundsException $ex) {
    echo "Caught ".$ex->getMessage()."\n";
  }
  var_dump($arr['foo'][1]);
  try {
    var_dump($arr['foo'][2]);
  } catch (OutOfBoundsException $ex) {
    echo "Caught ".$ex->getMessage()."\n";
  }
  $arr['foo'][3][] = 5;
  $arr['foo'][3]['bar'] = vec[12];
  try { var_dump($arr['foo'][3]['bar'][256]); }
  catch (Exception $e) { echo $e->getMessage()."\n"; }
  var_dump($arr);
}

<<__EntryPoint>> function main(): void {
  $foo = dict['a' => 'b', 'c' => 'd'];
  $foo[1] = 'INT-ONE';
  $foo['2'] = 'STR-TWO';
  $foo[3] = 'INT-THREE';
  $foo['4'] = 'STR-FOUR';

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

  $arr = dict['foo' => dict[1 => 'hello', '2' => 'world', 3 => dict[]]];
  manipulate($arr);
}
