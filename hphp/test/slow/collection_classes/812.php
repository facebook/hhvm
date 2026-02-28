<?hh


<<__EntryPoint>>
function main_812() :mixed{
$m = Map::fromArray(dict['a' => 'foo']);
var_dump($m['a']);
var_dump($m->at('a'));
var_dump($m->get('a'));
var_dump($m->get('b'));
}
