<?hh
namespace A;
const XX=1;
function fooBar() { echo __FUNCTION__ . \PHP_EOL; }

namespace B;
class A {
    static function fooBar() { echo "bag1\n"; }
}
class B {
    static function fooBar() { echo "bag2\n"; }
}
function fooBar() { echo __FUNCTION__ . \PHP_EOL; }
<<__EntryPoint>> function main(): void {
\var_dump(\A\XX);
A::fooBar();
\A\fooBar();
B::fooBar();
fooBar();
\B\fooBar();
}
