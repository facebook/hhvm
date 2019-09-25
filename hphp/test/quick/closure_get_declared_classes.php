<?hh
<<__EntryPoint>> function main(): void {
$a = function() {};
$b = function() {};
$classes = get_declared_classes();
asort(inout $classes);
foreach ($classes as $class) {
  if (stripos($class, 'Closure') !== FALSE) {
    var_dump($class);
  }
}
}
