<?hh

function f() { return 1; }

function foo(int $i) {
  if ($i) {
    $v = vec[1,1,1];
  } else {
    $v = vec[2,2,2];
  }

  $x = vec[$v];

  $a = $v[$i] ?? 42;
  $b = $x[$i][1] ?? 24;
  return vec[$a, $b];
}

var_dump(foo(10));
var_dump(foo(0));
