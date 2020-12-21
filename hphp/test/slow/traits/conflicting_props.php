<?hh

trait T {
    private $prop = 1;
}

class C {
  use T;
  protected static $prop = 2;
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
