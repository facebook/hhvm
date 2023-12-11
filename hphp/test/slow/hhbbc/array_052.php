<?hh

class A { public function yo() :mixed{ echo "hi\n"; } }

function foo() :mixed{
  $x = vec[vec[new A]];
  for ($i = 0; $i < 10; ++$i) {
    $x[] = vec[];
    $x[$i + 1][] = new A;
  }
  return $x;
}
function main() :mixed{
  $val = foo()[1][0];
  var_dump($val);
  $val->yo();
}

<<__EntryPoint>>
function main_array_052() :mixed{
main();
}
