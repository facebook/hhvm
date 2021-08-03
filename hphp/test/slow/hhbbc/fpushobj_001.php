<?hh
class C { public function heh() { echo "heh\n"; } }
function foo() {
  switch (HH\Lib\Legacy_FIXME\int_cast_for_switch(\HH\global_get('x'), null)) {
  case 0:
    return new C;
  }
}

function bar() {
  $x = foo();
  $x->heh();
  $x->heh();
  return $x;
}

function main() {
  $x = bar();
  var_dump($x);
}


<<__EntryPoint>>
function main_fpushobj_001() {
$x = 0;

main();
}
