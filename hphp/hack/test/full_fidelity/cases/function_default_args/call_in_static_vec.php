<?php

class C {
  public static function f() {
    return "lol";
  }
}

function f1($x = vec[C::f()]): void{}
