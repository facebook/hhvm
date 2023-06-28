<?hh

<<__EntryPoint>>
function main_invalid_setoption() :mixed{
$memcache = new \Memcached();
var_dump($memcache->setOption(-1, 'option'));
}
