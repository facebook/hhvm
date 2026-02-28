<?hh

class MyClass {
  public  function doStuff() :mixed{
    return function () {
      var_dump('called');
    };
  }
}


<<__EntryPoint>>
function main_method_returns_callable() :mixed{
(new MyClass())->doStuff()();
}
