<?hh

<<__EntryPoint>>
function main(): void {
  // should match
  var_dump(preg_match('/def/A', 'abcdef', 0, 3));
  // should not match
  var_dump(preg_match('/^def/', 'abcdef', 0, 3));
}
