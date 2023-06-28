<?hh
namespace Foo;
function f($a=Foo::bar) :mixed{
    return $a;
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
