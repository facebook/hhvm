<?hh



// should throw syntax errors

<<__EntryPoint>>
function main_ns_094() :mixed{
use const Foo\Bar\{
    A,
    const B,
    function C
}:mixed;
}
