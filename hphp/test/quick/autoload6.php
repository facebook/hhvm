<?hh
function my_autoload_func1($cls) {
  echo "my_autoload_func1 $cls\n";
}
function my_autoload_func2($cls) {
  echo "my_autoload_func2 $cls\n";
  $cls = strtolower($cls);
  if ($cls === 'i') {
    include 'autoload6-1.inc';
  }
}
function my_autoload_func3($cls) {
  echo "my_autoload_func3 $cls\n";
  $cls = strtolower($cls);
  if ($cls === 'i') {
    include 'autoload6-2.inc';
  }
}

class C { function __toString() { return 'I'; } }

<<__EntryPoint>> function main(): void {
  spl_autoload_register('my_autoload_func1');
  spl_autoload_register('my_autoload_func2');
  spl_autoload_register('my_autoload_func3');
  var_dump(interface_exists(new C));
}
