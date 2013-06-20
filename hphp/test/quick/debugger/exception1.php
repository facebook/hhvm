<?php

// Warning: line numbers are sensitive, do not change

namespace {

  class MyException extends Exception { }
  class NotAnException { }

  function throw_exception() {
    throw new Exception();
  }

  function throw_myexception() {
    throw new MyException();
  }

  function error_undefined_class() {
    $x = new NoSuchClass();
  }

}

namespace Outer {

  class MyException extends \Exception { }

  function throw_exception() {
    throw new \Exception();
  }

  function throw_myexception() {
    throw new MyException();
  }

  function error_undefined_class() {
    $x = new NoSuchClass();
  }

}

namespace Outer\Inner {

  class MyException extends \Exception { }

  function throw_exception() {
    throw new \Exception();
  }

  function throw_myexception() {
    throw new MyException();
  }

  function error_undefined_class() {
    $x = new NoSuchClass();
  }

}
