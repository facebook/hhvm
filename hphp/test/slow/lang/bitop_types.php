<?hh

class c {
  public function __toString()[] :mixed{
    return 'c';
  }
}

function test_uninit() :mixed{
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

function dump($a, $b, $bitop_str, $bitop) :mixed{
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
}

function cast(inout $a, inout $b) :mixed{
  if ($a is string && $b is string) return;
  $a = (int)$a;
  $b = (int)$b;
}

function and($a, $b) :mixed{
  cast(inout $a, inout $b);
  return $a & $b;
}

function xor($a, $b) :mixed{
  cast(inout $a, inout $b);
  return $a ^ $b;
}

function or($a, $b) :mixed{
  cast(inout $a, inout $b);
  return $a | $b;
}

<<__EntryPoint>>
function main_bitop_types() :mixed{
  $ops = vec[tuple('&', and<>), tuple('^', xor<>), tuple('|', or<>)];

  $values = vec[
    true,
    42,
    24.1987,
    "str",
    vec[1, 2, 3],
    new c(),
    null,
  ];

  foreach ($ops as list($op_str, $op_lambda)) {
    foreach ($values as $i) {
      foreach ($values as $j) {
        try {
          dump($i, $j, $op_str, $op_lambda);
        } catch (TypecastException $e) {
          var_dump($e->getMessage());
        }
      }
    }
  }

  @test_uninit();
}
