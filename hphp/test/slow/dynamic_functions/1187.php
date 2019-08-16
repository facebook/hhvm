<?hh
function foo() {
}
function goo(&$p) {
}
function bar() {
}

<<__EntryPoint>>
function main_1187() {
$goo = 'goo';
$foo = foo(); goo(&$foo);
$foo = foo(); $goo(&$foo);
bar(foo());
}
