<?hh

<<__EntryPoint>>
function main() {
  $a = "foo";
  var_dump(socket_select(inout $a, inout $a, inout $a, 42));
}
