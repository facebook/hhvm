<?php

$y = true;
define('foo', $y ? 1 : 0);
if (false) {
  class redecClass {
  }
}
 else {
  final class redecClass {
    const redecConst = foo;
    const redecConst2 = 456;
    public static $fx = foo;
  }
}
class T {
  const c = foo;
  const c2 = redecClass::redecConst;
  const c3 = redecClass::redecConst2;
  public static $q = foo;
  public static $n = 123;
}
class T2 {
  const c = foo;
  public static $q = foo;
}
class T3 {
  const c = foo;
}
class normal {
  const C = 1;
  public static $xx = 123;
}
function test() {
  var_dump(T::c);
  var_dump(T::c2);
  var_dump(T::c3);
  var_dump(T::$q);
  var_dump(T::$n);
  var_dump(T2::c);
  var_dump(T2::$q);
  var_dump(T3::c);
  var_dump(normal::C);
  var_dump(normal::$xx);
}
test();
