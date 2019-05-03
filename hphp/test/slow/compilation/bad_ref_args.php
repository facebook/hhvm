<?hh

<<__EntryPoint>>
function main() {
  $a = "foo";
  var_dump(socket_select(&$a, &$a, &$a, 42));
}
