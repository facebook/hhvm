<?php
class C {
  public function __toString() {
    return 123;
  }
}
function my_handler($errno, $errmsg) {
  echo "my_handler called\n";
  var_dump($errno);
  var_dump($errmsg);
}
function main() {
  set_error_handler('my_handler');
  $obj = new C;
  $str = (string)$obj;
  echo "Result: ";
  var_dump($str);
}
main();
