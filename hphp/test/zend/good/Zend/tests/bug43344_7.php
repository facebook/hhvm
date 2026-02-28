<?hh
namespace Foo;
function f($a=namespace\bar) :mixed{
    return $a;
}
<<__EntryPoint>> function main(): void {
echo f()."\n";
}
