<?hh
class C { public function heh() { echo "heh\n"; } }
function foo() {
  switch ($GLOBALS['x']) {
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
