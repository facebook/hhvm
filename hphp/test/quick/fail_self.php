<?hh
class X {
  static function test(self $s) {
    var_dump($s);
  }
}
<<__EntryPoint>> function main(): void {
X::test("hello");
}
