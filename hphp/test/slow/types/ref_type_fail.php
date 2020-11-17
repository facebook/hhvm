<?hh

function handler() {
 var_dump(__METHOD__);
 return true;
 }

function foo(<<__Soft>> inout varray $a) {
 var_dump($a);
 }

function test($a) {
  foo(inout $a);
}


<<__EntryPoint>>
function main_ref_type_fail() {
set_error_handler(handler<>);

test("hello");
test(varray[1,2,3]);
test(varray[]);
test("hello");
}
