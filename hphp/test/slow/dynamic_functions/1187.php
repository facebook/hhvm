<?hh
function foo() {
}
function goo(inout $p) {
}
function bar() {
}

<<__EntryPoint>>
function main_1187() {
$goo = 'goo';
$foo = foo(); goo(inout $foo);
$foo = foo(); $goo(inout $foo);
bar(foo());
}
