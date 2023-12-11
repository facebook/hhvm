<?hh
<<__EntryPoint>> function main(): void {
$m = new Memcached();
$m->addServer('localhost', 11211);

// getAllKeys doesn't seem to work until after another operation
$m->get('a');
$keys = $m->getAllKeys();
if ($keys) {
  array_map(function($key) use ($m) {
    $m->delete($key);
  }, $keys);
}

var_dump($m->getAllKeys());

$obj = new stdClass;
$obj->int = 99;
$obj->string = 'a simple string';
$obj->array = vec[11, 22];

$m->set('int', 99);
$m->set('string', 'a simple string');
$m->set('array', vec[11, 12]);
$m->set('object', $obj);

var_dump($m->getAllKeys());
}
