<?hh
namespace Foo;
function f($a=Foo::bar) {
    return $a;
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
