<?hh
namespace Foo;
function f($a=varray[Foo::bar]) {
    return $a[0];
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
