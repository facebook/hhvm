<?hh
class X {
  static function test(this $s) :mixed{
    var_dump($s);
  }
}
<<__EntryPoint>> function main(): void {
X::test("hello");
}
