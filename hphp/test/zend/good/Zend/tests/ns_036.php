<?hh
namespace A;
use A as B;
class MultipleIterator {
    const MIT_NEED_ALL = 2;
}
function f1($x = MultipleIterator::MIT_NEED_ALL) {
    \var_dump($x);
}
function f2($x = \MultipleIterator::MIT_NEED_ALL) {
    \var_dump($x);
}
function f3($x = \A\MultipleIterator::MIT_NEED_ALL) {
    \var_dump($x);
}
function f4($x = B\MultipleIterator::MIT_NEED_ALL) {
    \var_dump($x);
}
<<__EntryPoint>> function main(): void {
\var_dump(MultipleIterator::MIT_NEED_ALL);
\var_dump(\MultipleIterator::MIT_NEED_ALL);
\var_dump(B\MultipleIterator::MIT_NEED_ALL);
\var_dump(\A\MultipleIterator::MIT_NEED_ALL);
f1();
f2();
f3();
f4();
}
