<?hh

<<__EntryPoint>>
function main () {
  $a = darray["" => "empty string", 1 => "one", 0 => "zero"];
  var_dump($a + darray["more stuff" => "yep"]);
}
