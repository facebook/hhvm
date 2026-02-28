<?hh
// Similar case, but for a builtin (array_multisort).
function blarg2(inout $a1, inout $a2) :mixed{}

// array_multisort is weird.  Some arguments are literals.
function main4() :mixed{
  $x = vec[1, 54, 3, 23, 5, 2];
  $y = vec["a", "b", "c", "d", "e", "f"];
  var_dump($x, $y);
  array_multisort2(inout $x, inout $y);
  var_dump($x, $y);
  $desc = SORT_DESC;
  array_multisort3(inout $x, inout $desc, inout $y);
  var_dump($x, $y);
}

function main5() :mixed{
  $params = vec[vec[3,2,1],vec[4,6,5],vec[7,9,8]];
  array_multisort3(...$params);
  var_dump($params);
}



// Tests a case where we are passing more args than a function takes
// to its reffiness guard.
<<__EntryPoint>>
function main_ref_args() :mixed{
error_reporting(0);
main4();
main5();
}
