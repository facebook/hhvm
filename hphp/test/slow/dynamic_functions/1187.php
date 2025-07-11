<?hh
function foo() :mixed{
}
<<__DynamicallyCallable>>
function goo(inout $p) :mixed{
}
function bar() :mixed{
}

<<__EntryPoint>>
function main_1187() :mixed{
$goo = 'goo';
$foo = foo(); goo(inout $foo);
$foo = foo(); HH\dynamic_fun($goo)(inout $foo);
bar(foo());
}
