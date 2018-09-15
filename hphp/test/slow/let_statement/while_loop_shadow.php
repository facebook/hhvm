<?hh // experimental

$b = true;
let x = "x";
while ($b) {
  var_dump(x);
  let x = 42;
  var_dump(x);
  $b = false;
}
var_dump(x);
