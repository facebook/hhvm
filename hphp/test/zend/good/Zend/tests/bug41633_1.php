<?hh
class Foo {
    const A = self::B;
    const B = "ok";
}
<<__EntryPoint>> function main(): void {
echo Foo::A."\n";
}
