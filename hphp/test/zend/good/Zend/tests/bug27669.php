<?hh

class A {
  static function hello(): void {
    echo "Hello World\n";
  }
}

<<__EntryPoint>> function main(): void {
  error_reporting(E_ALL & ~E_STRICT);
  $y = vec['hello'];
  A::$y[0]();
  echo "===DONE===\n";
}
