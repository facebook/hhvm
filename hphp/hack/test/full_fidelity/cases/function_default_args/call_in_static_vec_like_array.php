<?php

class C {
  public static function f() {
    return "lol";
  }
}

function f($x = array(C::f())): void{}
