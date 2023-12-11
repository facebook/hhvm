<?hh


<<__EntryPoint>>
function main_811() :mixed{
$v = Vector::fromArray(vec['foo']);
var_dump($v[0]);
var_dump($v->at(0));
var_dump($v->get(0));
var_dump($v->get(1));
$m = Map::fromArray(vec['foo']);
var_dump($m[0]);
var_dump($m->at(0));
var_dump($m->get(0));
var_dump($m->get(1));
}
