<?hh

function main() {
  $v = null;
  hit1(inout $v);
  var_dump($v);
  hit2(inout $v);
  var_dump($v);
  hit3(inout $v);
  var_dump($v);
  partial(inout $v); // error
}

<<__EntryPoint>>
function entrypoint_autoloader(): void {
  main();
}
