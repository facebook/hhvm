<?hh

class MyClass {
  public  function doStuff() {
    return function () {
      var_dump('called');
    };
  }
}


<<__EntryPoint>>
function main_method_returns_callable() {
(new MyClass())->doStuff()();
}
