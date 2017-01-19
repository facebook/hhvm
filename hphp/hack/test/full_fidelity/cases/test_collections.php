<?hh
function foo() {
  vec(); // Legal!
  $x = vec[1, 2];
  $y = dict["abc" => $x];
  $z = keyset[3, 4];
  $a = vec[];
}
