<?hh

class c {
  public function __toString() {
    return 'c';
  }
}

function test_uninit() {
  if (false) {}
  $a = 'string';
  try {
    $x = $a & $b;
    var_dump($x);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }

  try {
    $x = $a ^ $b;
    var_dump($x);
    var_dump($x);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_bitop_types() {
  $func = ($a, $b, $bitop_str, $bitop) ==> {
    $res = $bitop($a, $b);
    printf(
      "%s(%s) %s %s(%s) = %s(%s)\n",
      gettype($a),
      HH\is_any_array($a) ? 'Array' : $a,
      $bitop_str,
      gettype($b),
      HH\is_any_array($b) ? 'Array' : $b,
      gettype($res),
      $res,
    );
  };

  $ops = varray[
    tuple('&', ($a, $b) ==> $a & $b),
    tuple('^', ($a, $b) ==> $a ^ $b),
    tuple('|', ($a, $b) ==> $a | $b),
  ];

  $values = varray[
    true,
    42,
    24.1987,
    "str",
    varray[1, 2, 3],
    new c(),
    null,
  ];

  for ($o = 0; $o < count($ops); ++$o) {
    for ($i = 0; $i < count($values); ++$i) {
      for ($j = 0; $j < count($values); ++$j) {
        list($op_str, $op_lambda) = $ops[$o];
        try {
          $func($values[$i], $values[$j], $op_str, $op_lambda);
        } catch (TypecastException $e) {
          var_dump($e->getMessage());
        }
      }
    }
  }

  @test_uninit();
}
