<?hh
<<__EntryPoint>> function main(): void {
$memc = new Memcached();

$servers = varray[
  // Test both servers formats
  darray['host' => 'localhost', 'port' => 11211, 'weight' => 50],
  varray['localhost', 22222, 50] // Dummy port to check failure
];

$memc->addServers($servers);
var_dump($memc->getVersion());
}
