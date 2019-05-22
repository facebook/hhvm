<?hh
function test($t) {
  unset($GLOBALS['foo']['bar']);
}
<<__EntryPoint>> function main(): void {
test(null);
}
