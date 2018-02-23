<?php

class MyClass {
  public  function doStuff() {
    return function () {
      var_dump('called');
    };
  }
}

(new MyClass())->doStuff()();
