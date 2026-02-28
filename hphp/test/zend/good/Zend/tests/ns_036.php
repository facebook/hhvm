<?hh
namespace A;
use A as B;
class RegexIterator {
    const GET_MATCH = 2;
}
function f1($x = RegexIterator::GET_MATCH) :mixed{
    \var_dump($x);
}
function f2($x = \RegexIterator::GET_MATCH) :mixed{
    \var_dump($x);
}
function f3($x = \A\RegexIterator::GET_MATCH) :mixed{
    \var_dump($x);
}
function f4($x = B\RegexIterator::GET_MATCH) :mixed{
    \var_dump($x);
}
<<__EntryPoint>> function main(): void {
\var_dump(RegexIterator::GET_MATCH);
\var_dump(\RegexIterator::GET_MATCH);
\var_dump(B\RegexIterator::GET_MATCH);
\var_dump(\A\RegexIterator::GET_MATCH);
f1();
f2();
f3();
f4();
}
