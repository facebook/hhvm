<?php

class S {
  public static function t() {
    echo func_get_arg(0);
  }
}
class C {
  public static function d() {
    echo static::class;
    S::t(static::class);
    echo static::class;
  }
}

<<__EntryPoint>>
function main_1884() {
C::d();
}
