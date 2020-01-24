<?hh
namespace A;
use A as B;
class ArrayIterator {
    const STD_PROP_LIST = 2;
}
function f1($x = ArrayIterator::STD_PROP_LIST) {
    \var_dump($x);
}
function f2($x = \ArrayIterator::STD_PROP_LIST) {
    \var_dump($x);
}
function f3($x = \A\ArrayIterator::STD_PROP_LIST) {
    \var_dump($x);
}
function f4($x = B\ArrayIterator::STD_PROP_LIST) {
    \var_dump($x);
}
<<__EntryPoint>> function main(): void {
\var_dump(ArrayIterator::STD_PROP_LIST);
\var_dump(\ArrayIterator::STD_PROP_LIST);
\var_dump(B\ArrayIterator::STD_PROP_LIST);
\var_dump(\A\ArrayIterator::STD_PROP_LIST);
f1();
f2();
f3();
f4();
}
