<?hh
<<__EntryPoint>> function main(): void {
$memcache = new Memcache();
$memcache->addServer('localhost', 11211);

$keys = vec[
  'abcdef1',
  'abcdef11',
  'abcdef12',
  'abcdef13',
];

foreach($keys as $k) {
  $memcache->set($k, "value:" . $k);
}
$r = $memcache->get($keys);
var_dump($r);
}
