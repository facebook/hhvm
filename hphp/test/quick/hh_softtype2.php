<?hh

interface Hey {
  function wat(<<__Soft>> Foo $x);
}

class Bar implements Hey {
  public function wat($x) {
  }
}
<<__EntryPoint>> function main(): void {
new Bar();
echo "ok\n";
}
