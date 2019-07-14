<?hh
class Foo<a,b,c> {
  static function test1<b,c,d>(b $x) {
  }
  static function test2<b,c,d>(d $x) {
  }
  static function test3<x,y,z>(c<foozle> $x) {
    // only base name matters
  }
  static function test4(string<foo,bar> $x) {
    // primitives are ok
  }
  static function test5(d $x) {
    // this should fail
  }
}
function test6<foo>(foo $x) : garbage { }
function test7<foo>(string $x) { }

<<__EntryPoint>> function main(): void {
Foo::test1('foo');
Foo::test2('foo');
Foo::test3('foo');
Foo::test4('foo');
Foo::test5('foo');
test6('foo');
test7('foo');
echo "failed";
}
