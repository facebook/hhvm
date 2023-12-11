<?hh
<<__EntryPoint>> function main(): void {
$memc = new Memcached();

$servers = vec[
  // Test both servers formats
  dict['host' => 'localhost', 'port' => 11211, 'weight' => 50],
  vec['localhost', 22222, 50] // Dummy port to check failure
];

$memc->addServers($servers);
var_dump($memc->getVersion());
}
