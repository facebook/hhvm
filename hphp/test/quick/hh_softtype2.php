<?hh

interface Hey {
  function wat(@Foo $x);
}

class Bar implements Hey {
  public function wat($x) {
  }
}

new Bar();
echo "ok\n";
