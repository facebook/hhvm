<?hh

class C { public function heh() :mixed{ echo "hey\n"; } }
function foo() :mixed{
  $x = vec[];
  for ($i = 0; $i < 10; ++$i) {
    $x[] = new C;
  }
  return $x;
}
function main() :mixed{
  $x = foo();
  $x[0]->heh();
}

<<__EntryPoint>>
function main_array_056() :mixed{
main();
}
