<?php

class C {
  public static function f() {
    return "lol";
  }
}

function f($x = varray[C::f()]): void{}
