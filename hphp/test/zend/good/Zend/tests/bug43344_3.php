<?hh
namespace Foo;
function f($a=Foo::bar) {
    return $a;
}
<<__EntryPoint>> function main() {
echo f()."\n";
}
