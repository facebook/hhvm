<?hh

class MyClass {
  public  function doStuff() {
    return function () {
      var_dump('outer');
      return function() {
          var_dump('inner');
      };
    };
  }
}


<<__EntryPoint>>
function main_method_returns_callable_chained() {
(new MyClass())->doStuff()()();
}
