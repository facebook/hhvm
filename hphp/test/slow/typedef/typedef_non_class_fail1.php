<?hh

type MyString = string;

function foo(MyString $x): void {
}

function test() :mixed{
  foo(123); // failure, expected string
}

<<__EntryPoint>>
function main_typedef_non_class_fail1() :mixed{
test();
}
