<?hh

interface Hey {
  function wat(<<__Soft>> Foo $x):mixed;
}

class Bar implements Hey {
  public function wat($x) :mixed{
  }
}
<<__EntryPoint>> function main(): void {
new Bar();
echo "ok\n";
}
