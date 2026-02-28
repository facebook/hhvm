<?hh


<<__EntryPoint>>
function main_reset_server_list() :mixed{
$mem = new Memcached();
$mem->addServer("localhost", 1234);
var_dump(count($mem->getServerList()));
$mem->resetServerList();
var_dump($mem->getServerList());
}
