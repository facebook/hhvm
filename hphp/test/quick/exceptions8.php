<?hh
function doThrow() {
  throw new Exception("f you!");
}
function f($a1, $a2) {
  if (false) {
    var_dump($b1);
    var_dump($b2);
    var_dump($b3);
    var_dump($b4);
    var_dump($b5);
    var_dump($b6);
    var_dump($b7);
    var_dump($b8);
    var_dump($b9);
    var_dump($b10);
  }
}
function main() {
  try {
    f(1);
  } catch (Exception $e) {
  }
  echo "Done\n";
}
function main2() {
  try {
    new NonExist();
  } catch (Exception $e) {
  }
}
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);
  set_error_handler(fun('doThrow'));
  main();

  spl_autoload_register('doThrow');
  main2();
}
