<?php
function my_autoload_func1($cls) {
  echo "my_autoload_func1 $cls\n";
}
function my_autoload_func2($cls) {
  echo "my_autoload_func2 $cls\n";
  $cls = strtolower($cls);
  if ($cls === 'i') {
    interface I {
      public function foo();
    }
  }
}
function my_autoload_func3($cls) {
  echo "my_autoload_func3 $cls\n";
  $cls = strtolower($cls);
  if ($cls === 'i') {
    interface I {
      public function bar();
    }
  }
}
function main() {
  spl_autoload_register('my_autoload_func1');
  spl_autoload_register('my_autoload_func2');
  spl_autoload_register('my_autoload_func3');
  var_dump(interface_exists('I'));
}
main();

