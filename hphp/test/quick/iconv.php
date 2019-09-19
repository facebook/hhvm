<?hh

<<__EntryPoint>>
function main(): void {
  // This test is a regression test for t48750198.
  var_dump(iconv('utf8', 'ascii//TRANSLIT', 'my test string'));
}
