<?hh

<<A(1)>>
function f() :mixed{
}

<<__EntryPoint>>
function main_2199() :mixed{
$rf = new ReflectionFunction('f');
var_dump($rf->getAttribute('A'));
var_dump($rf->getAttribute('B'));
var_dump($rf->getAttributes());
}
