<?hh


<<__EntryPoint>>
function main_812() {
$m = Map::fromArray(darray['a' => 'foo']);
var_dump($m['a']);
var_dump($m->at('a'));
var_dump($m->get('a'));
var_dump($m->get('b'));
}
