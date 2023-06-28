<?hh


<<__EntryPoint>>
function main_quit() :mixed{
$m = new Memcached();
var_dump($m->quit());
}
