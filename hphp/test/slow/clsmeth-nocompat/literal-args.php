<?hh

class A {
  static function test() {
    var_dump(__METHOD__);
    class_meth(__CLASS__, __FUNCTION__);
    class_meth(__CLASS__, 'test');
    class_meth('A', __FUNCTION__);
    class_meth(self::class, 'test');
    class_meth(static::class, 'test');
  }
}
class B extends A {
  static function test() {
    var_dump(__METHOD__);
    class_meth(parent::class, 'test')();
  }
}

trait T {
  <<__MemoizeLSB>>
  static function test() {
    var_dump(static::class.'::'.__FUNCTION__);
    return ($x ==> { echo var_export($x, true)."\n"; return $x; })(
      class_meth(__CLASS__, __FUNCTION__),
    );
  }
}
class C { use T; }
class D extends C {}

<<__EntryPoint>>
function main() {
  class_meth('A', 'test');
  class_meth(A::class, 'test');

  $inv = ($x, $n) ==> { while ($n-- > 0) $x = $x(); echo "---\n"; };
  $inv(class_meth(B::class, 'test'), 1);
  $inv(class_meth(T::class, 'test'), 2);
  $inv(class_meth(D::class, 'test'), 3);
  $inv(class_meth(C::class, 'test'), 10);

  echo "Done\n";
}
