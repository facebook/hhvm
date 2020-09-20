<?hh


<<__EntryPoint>>
function main_bad_call_1() {
foo(inout $a = func());
}
