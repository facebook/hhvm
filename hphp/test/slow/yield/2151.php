<?hh

function f(...$args) :AsyncGenerator<mixed,mixed,void>{
 yield count($args);
 yield $args[1];
 }

<<__EntryPoint>>
function main_2151() :mixed{
foreach (f(1, 2, 3) as $v) {
 var_dump($v);
 }
}
