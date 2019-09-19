<?hh
namespace Foo;
function f($a=array(namespace\bar=>0)) {
    reset(inout $a);
    return key($a);
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
