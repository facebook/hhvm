<?hh


<<__EntryPoint>>
function main_quit() {
$m = new Memcached();
var_dump($m->quit());
}
