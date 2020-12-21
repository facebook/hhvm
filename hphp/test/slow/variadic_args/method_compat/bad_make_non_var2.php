<?hh

interface DB {
  public function query($query, ...$params);
}

class MySQL implements DB {
  public function query($query, ?Foo $param = null) { }
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
