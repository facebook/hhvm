<?php

// Wanring: line numbers are sensitive, do not change

class MyException extends Exception { }

function throw_exception() {
  throw new Exception();
}

function throw_myexception() {
  throw new MyException();
}

function error_undefined_class() {
  $x = new NoSuchClass();
}
