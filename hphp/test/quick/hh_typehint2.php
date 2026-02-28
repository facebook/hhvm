<?hh

interface Hey {
  function wat(?Foo $x):mixed;
}

class Bar implements Hey {
  public function wat($x) :mixed{
  }
}
<<__EntryPoint>> function main(): void {
new Bar();
echo "ok\n";
}
