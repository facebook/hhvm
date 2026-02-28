<?hh

<<__EntryPoint>>
function foo() :mixed{
  var_dump(substr_compare("\x00", "\x00\x00\x00\x00\x00\x00\x00\x00", 0, 65535, false));
}
