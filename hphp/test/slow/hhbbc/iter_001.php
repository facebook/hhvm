<?hh

class C { function heh() :mixed{ echo "hi\n"; } }
function foo() :mixed{
  $x = vec[new C, new C];
  foreach ($x as $v) {
    $v->heh();
  }
}

<<__EntryPoint>>
function main_iter_001() :mixed{
foo();
}
