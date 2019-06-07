<?hh
class Foo {
    const A = self::B;
}
<<__EntryPoint>> function main() {
echo Foo::A."\n";
}
