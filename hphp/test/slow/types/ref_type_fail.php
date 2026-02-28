<?hh

function handler() :mixed{
 var_dump(__METHOD__);
 return true;
 }

function foo(<<__Soft>> inout varray $a) :mixed{
 var_dump($a);
 }

function test($a) :mixed{
  foo(inout $a);
}


<<__EntryPoint>>
function main_ref_type_fail() :mixed{
set_error_handler(handler<>);

test("hello");
test(vec[1,2,3]);
test(vec[]);
test("hello");
}
