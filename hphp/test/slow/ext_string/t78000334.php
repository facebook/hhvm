<?hh

<<__EntryPoint>>
function foo() {
  var_dump(substr_compare("\x00", "\x00\x00\x00\x00\x00\x00\x00\x00", 0, 65535, false));
}
