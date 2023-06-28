<?hh
const C = "const ok\n";

function foo() :mixed{
    return "func ok\n";
}

class foo {
    const C = "const ok\n";
    const C2 = namespace\C;
    public static $var = "var ok\n";
    function __construct() {
        echo "class ok\n";
    }
    static function bar() :mixed{
        return "method ok\n";
    }
}

function f1($x=namespace\C) :mixed{
    return $x;
}
function f2($x=namespace\foo::C) :mixed{
    return $x;
}

function f3(namespace\foo $x) :mixed{
    return "ok\n";
}
<<__EntryPoint>> function main(): void {
echo namespace\C;
echo namespace\foo();
echo namespace\foo::C;
echo namespace\foo::C2;
echo namespace\foo::$var;
echo namespace\foo::bar();
echo namespace\f1();
echo namespace\f2();
echo namespace\f3(new namespace\foo());
echo namespace\unknown;
}
