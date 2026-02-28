<?hh

class Foo {
  public function bar() :mixed{
    (function($x) { var_dump('baz'); })(5);
    (($x) ==> var_dump('biz'))(5);
  }
}


<<__EntryPoint>>
function main_closure_invoke() :mixed{
(new Foo())->bar();
}
