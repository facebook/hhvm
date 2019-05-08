<?hh
class X {
  static function test(self $s) {
    var_dump($s);
  }
}
X::test("hello");
