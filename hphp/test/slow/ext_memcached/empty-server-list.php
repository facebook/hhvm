<?hh

<<__EntryPoint>>
function main_empty_server_list() :mixed{
$memcache = new Memcached();
var_dump($memcache->getServerList());
}
