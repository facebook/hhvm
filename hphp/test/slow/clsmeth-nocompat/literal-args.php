<?hh

class A {
  static function test() {
    var_dump(__METHOD__);
    class_meth(__CLASS__, __FUNCTION__);
    A::test<>;
    class_meth('A', __FUNCTION__);
    self::test<>;
    static::test<>;
  }
}
class B extends A {
  static function test() {
    var_dump(__METHOD__);
    (parent::test<>)();
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
  A::test<>;
  A::test<>;

  $inv = ($x, $n) ==> { while ($n-- > 0) $x = $x(); echo "---\n"; };
  $inv(B::test<>, 1);
  $inv(T::test<>, 2);
  $inv(D::test<>, 3);
  $inv(C::test<>, 10);

  echo "Done\n";
}
