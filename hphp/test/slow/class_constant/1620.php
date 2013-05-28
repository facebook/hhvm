<?php

class W {
  const FOO = 0;
}
class X extends W {
  const FOO = 1;
  static function foo() {
    var_dump(constant('self::FOO'));
    var_dump(constant('parent::FOO'));
    var_dump(constant('static::FOO'));
    var_dump(defined('self::FOO'));
    var_dump(defined('parent::FOO'));
    var_dump(defined('static::FOO'));
    var_dump(defined('self::BAR'));
    var_dump(defined('parent::BAR'));
    var_dump(defined('static::BAR'));
 }
}
class Y extends X {
  const FOO = 2;
  const BAR = 1;
}
X::foo();
Y::foo();
