<?hh

<<__EntryPoint>>
function main_invalid_setoption() {
$memcache = new \Memcached();
var_dump($memcache->setOption(-1, 'option'));
}
