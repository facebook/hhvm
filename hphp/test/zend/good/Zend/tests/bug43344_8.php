<?hh
namespace Foo;
function f($a=varray[namespace\bar]) :mixed{
    return $a[0];
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
