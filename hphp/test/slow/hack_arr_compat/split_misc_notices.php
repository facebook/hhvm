<?hh

<<__EntryPoint>>
function main () {
  $a = darray["" => "empty string", 1 => "one", 0 => "zero"];
  var_dump($a + darray["more stuff" => "yep"]);
  var_dump($a[null]);
  var_dump($a[true]);
  var_dump($a[0.0]);
}
