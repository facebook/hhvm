<?hh
abstract class base {
  public $a = 'base';

  // disallow cloning once forever
  final private function __clone() {}
}

class test extends base {
  // reenabling should fail
  public function __clone() {}
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
