<?hh

interface Hey {
  function wat(@Foo $x);
}

class Bar implements Hey {
  public function wat($x) {
  }
}
<<__EntryPoint>> function main(): void {
new Bar();
echo "ok\n";
}
