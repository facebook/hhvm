<?hh

function foo($a, $b, $c) { var_dump($a, $b, $c); }

class X {
  const FOO=array(1,2,3);
  static function bar() {
    foo(...self::FOO);
  }
}
X::bar();
