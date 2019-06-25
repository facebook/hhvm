<?hh
namespace Foo;
function f($a=array(Foo::bar=>0)) {
    reset(&$a);
    return key($a);
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
