<?hh

function dump_types($v) :mixed{
  echo gettype($v) . "\n";
}
<<__EntryPoint>>
function main_entry(): void {

  $m = new Memcached();
  $m->addServer('localhost', '11211');

  $keys = vec[100, 'str'];
  foreach ($keys as $key) {
    dump_types($key);
  }

  $deleted = $m->deleteMulti($keys);
  foreach ($keys as $key) {
    dump_types($key);
  }
}
