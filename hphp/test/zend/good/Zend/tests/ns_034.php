<?hh
namespace A;
use A as B;
class Foo {
    const C = "ok\n";
}
function f1($x=Foo::C) :mixed{
    echo $x;
}
function f2($x=B\Foo::C) :mixed{
    echo $x;
}
function f3($x=\A\Foo::C) :mixed{
    echo $x;
}
<<__EntryPoint>> function main(): void {
echo Foo::C;
echo B\Foo::C;
echo \A\Foo::C;
f1();
f2();
f3();
}
