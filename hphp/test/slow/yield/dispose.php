<?hh

class foo {
  public function __dispose() {
    yield foo();
  }
}

