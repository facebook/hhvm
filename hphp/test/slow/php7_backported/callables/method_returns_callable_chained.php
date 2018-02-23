<?php

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

(new MyClass())->doStuff()()();
