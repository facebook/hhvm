<?hh

class Foo {
  public function bar() {
    (function($x) { var_dump('baz'); })(5);
    (($x) ==> var_dump('biz'))(5);
  }
}

(new Foo())->bar();
