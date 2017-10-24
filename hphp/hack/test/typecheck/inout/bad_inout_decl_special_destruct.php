<?hh

class C {
  public function __destruct(inout $_) {
    echo 'Destructing '.__CLASS__."\n";
  }
}
