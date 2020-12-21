<?hh
abstract class bar {
    abstract public function bar();
}

class foo extends bar {
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
