<?hh

class X {
 const FOO = 'hello';
 }
function foo(&$a) {
 static $s;
 }

<<__EntryPoint>>
function main_1319() {
if (class_exists('X')) foo(X::FOO);
}
