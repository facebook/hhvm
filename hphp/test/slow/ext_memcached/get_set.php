<?hh
<<__EntryPoint>> function main(): void {
$memc = new Memcached();
$memc->addServer('localhost', '11211');

$key = 'foo';
$value = dict['foo' => 'bar'];

$memc->set($key, $value, 60);
var_dump($memc->get($key));
}
