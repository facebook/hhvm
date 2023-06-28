<?hh
function foo1() :mixed{
  $x = "ab";
  $y = "c";
  return $x . $y;
}

function foo2() :mixed{
  $x = "x";
  $y = "y";
  $z = "z";

  return $x . $y . $z;
}

function foo3($x) :mixed{
  $y = "c";
  return $x . $y;
}
<<__EntryPoint>> function main(): void {
var_dump(foo1());
var_dump(foo2());
var_dump(foo3("ab"));
}
