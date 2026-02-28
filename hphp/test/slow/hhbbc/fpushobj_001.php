<?hh
class C { public function heh() :mixed{ echo "heh\n"; } }
function foo() :mixed{
  switch (HH\Lib\Legacy_FIXME\int_cast_for_switch(\HH\global_get('x'), null)) {
  case 0:
    return new C;
  }
}

function bar() :mixed{
  $x = foo();
  $x->heh();
  $x->heh();
  return $x;
}

function main() :mixed{
  $x = bar();
  var_dump($x);
}


<<__EntryPoint>>
function main_fpushobj_001() :mixed{
$x = 0;

main();
}
