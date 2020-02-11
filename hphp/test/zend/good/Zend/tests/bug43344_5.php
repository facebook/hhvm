<?hh
namespace Foo;
function f($a=darray[Foo::bar=>0]) {
    reset(inout $a);
    return key($a);
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
