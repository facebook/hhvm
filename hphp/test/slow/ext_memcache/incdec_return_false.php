<?hh <<__EntryPoint>> function main(): void {
$key = 'TEST_KEY_INCREMENT_DECREMENT';
$memcache = new Memcache;
$memcache->addServer('127.0.0.1', 11211);

$memcache->delete($key);
var_dump($memcache->increment($key, 1));
var_dump($memcache->get($key));

$memcache->set($key, 0);
var_dump($memcache->increment($key, 1));

$memcache->delete($key);
var_dump($memcache->decrement($key, 1));
var_dump($memcache->get($key));

$memcache->set($key, 0);
var_dump($memcache->decrement($key, 1));

$memcache->delete($key);
}
