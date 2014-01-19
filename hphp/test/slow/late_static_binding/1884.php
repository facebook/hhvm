<?php

class S {
  public static function t() {
    echo func_get_arg(0);
  }
}
class C {
  public static function d() {
    echo get_called_class();
    S::t(get_called_class());
    echo get_called_class();
  }
}
C::d();
