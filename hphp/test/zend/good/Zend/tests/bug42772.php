<?hh
class Foo {
    static public $foo;
    function __toString() :mixed{
        self::$foo = $this;
        return 'foo';
    }
}
<<__EntryPoint>> function main(): void {
$foo = (string)new Foo();
var_dump(Foo::$foo);
}
