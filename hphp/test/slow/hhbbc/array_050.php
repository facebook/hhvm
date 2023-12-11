<?hh

function a() :mixed{ return 1; }
function foo() :mixed{
  $x = vec[a()];
  $x[] = 0;
  $x[1]++;
  return $x;
}
function d() :mixed{
  $y = foo();
  var_dump($y);
}

<<__EntryPoint>>
function main_array_050() :mixed{
d();
}
