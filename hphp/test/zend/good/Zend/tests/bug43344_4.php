<?hh
namespace Foo;
function f($a=vec[Foo::bar]) :mixed{
    return $a[0];
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
