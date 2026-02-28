<?hh

class Bar {
    public static function pub() :mixed{
        Bar::priv();
    }
    private static function priv()    :mixed{
        echo "Bar::priv()\n";
    }
}
class Foo extends Bar {
    public static function priv()    :mixed{
        echo "Foo::priv()\n";
    }
}
<<__EntryPoint>> function main(): void {
Foo::pub();
Foo::priv();

echo "Done\n";
}
