<?hh

type MyString = string;

function foo(MyString $x): void {
}

function test() {
  foo(123); // failure, expected string
}
test();