<?hh


<<__EntryPoint>>
function main_bad_call_1() :mixed{
foo(inout $a = func());
}
