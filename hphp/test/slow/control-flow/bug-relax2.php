<?hh

function foo($arr) {
  $children = varray[];
  foreach ($arr as $child) {
    $children[] = $child;
  }
  return $children;
}


<<__EntryPoint>>
function main_bug_relax2() {
var_dump(foo(varray[]));
var_dump(foo(varray[1]));
var_dump(foo(varray[2]));
}
