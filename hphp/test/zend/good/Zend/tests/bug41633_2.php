<?hh
class Foo {
    const A = self::B;
}
<<__EntryPoint>> function main(): void {
echo Foo::A."\n";
}
