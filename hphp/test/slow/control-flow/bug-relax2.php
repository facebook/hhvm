<?hh

function foo($arr) :mixed{
  $children = vec[];
  foreach ($arr as $child) {
    $children[] = $child;
  }
  return $children;
}


<<__EntryPoint>>
function main_bug_relax2() :mixed{
var_dump(foo(vec[]));
var_dump(foo(vec[1]));
var_dump(foo(vec[2]));
}
