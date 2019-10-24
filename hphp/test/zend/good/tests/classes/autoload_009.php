<?hh

function __autoload($name) {
  echo "In autoload: ";
  var_dump($name);
}

function f(UndefClass $x) {}

<<__EntryPoint>> function main(): void {
  f(new stdClass());
}
