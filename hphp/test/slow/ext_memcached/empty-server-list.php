<?hh

<<__EntryPoint>>
function main_empty_server_list() {
$memcache = new Memcached();
var_dump($memcache->getServerList());
}
