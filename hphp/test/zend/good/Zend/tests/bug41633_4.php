<?hh
class Foo {
    const A = self::B;
    const B = "ok";
}
<<__EntryPoint>> function main(): void {
var_dump(defined("Foo::A"));
}
