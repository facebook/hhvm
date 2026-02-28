<?hh

class foo {
  public function __dispose() :AsyncGenerator<mixed,mixed,void>{
    yield foo();
  }
}

