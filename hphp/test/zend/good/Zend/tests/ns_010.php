<?hh
namespace X;
use X as Y;
class Foo {
    const C = "const ok\n";
    public static $var = "var ok\n";
    function __construct() {
        echo "class ok\n";
    }
    static function bar() :mixed{
        echo "method ok\n";
    }
}
<<__EntryPoint>> function main(): void {
new Foo();
new Y\Foo();
new \X\Foo();
Foo::bar();
Y\Foo::bar();
\X\Foo::bar();
echo Foo::C;
echo Y\Foo::C;
echo \X\Foo::C;
echo Foo::$var;
echo Y\Foo::$var;
echo \X\Foo::$var;
}
