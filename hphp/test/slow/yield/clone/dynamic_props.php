<?hh

function gen() :AsyncGenerator<mixed,mixed,void>{ yield 7; yield 8; }

<<__EntryPoint>>
function main_dynamic_props() :mixed{
$g = gen();
$g->lol = "whut";
var_dump($g);
$h = clone($g);
var_dump($h);
}
