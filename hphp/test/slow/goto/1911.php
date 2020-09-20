<?hh

function foo() {
 goto a;
 b: echo 'Foo';
 return;
a: echo 'Bar';
 goto b;
}

 <<__EntryPoint>>
function main_1911() {
foo();
}
