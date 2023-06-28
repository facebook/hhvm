<?hh
class Foo {
    public function __construct() {
        eval("class Bar extends Foo {}");
        Some::foo($this);
    }
}
class Some {
    public static function foo(Foo $foo) :mixed{
    }
}
<<__EntryPoint>> function main(): void {
new Foo;
echo "done\n";
}
