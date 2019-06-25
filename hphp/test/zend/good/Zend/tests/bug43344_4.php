<?hh
namespace Foo;
function f($a=array(Foo::bar)) {
    return $a[0];
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
