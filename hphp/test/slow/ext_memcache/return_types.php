<?hh
<<__EntryPoint>> function main(): void {
$key = 'TEST_KEY_RETURN_TYPES';

$memcache = new Memcache;
$memcache->addServer('127.0.0.1', 11211);

$memcache->delete($key);

$stdClass = new stdClass();
$stdClass->prop1 = 1;
$stdClass->prop2 = '2';

$testValueList = vec[
  'Some string',
  '1234567',
  1234567,
  '120.88',
  120.88,
  'true',
  'false',
  true,
  false,
  $stdClass,
  'null',
  null
];

foreach ($testValueList as $value) {
  $memcache->set($key, $value);
  var_dump($memcache->get($key));
}

$memcache->delete($key);
}
