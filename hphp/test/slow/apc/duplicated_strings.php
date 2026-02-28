<?hh

<<__EntryPoint>>
function main(): void {
  $value = vec[];
  for ($i = 0; $i < 100000; $i++) {
    $value[] = 'stringstringstringstringstring'.$i;
  }

  apc_store('foo_different', $value);

  $value = vec[];
  for ($i = 0; $i < 100000; $i++) {
    $value[] = 'stringstringstringstringstring'.__hhvm_intrinsics\launder_value(1);
  }

  apc_store('foo_unique', $value);

  $value = vec[];
  $str = 'stringstringstringstringstring'.__hhvm_intrinsics\launder_value(1);
  for ($i = 0; $i < 100000; $i++) {
    $value[] = $str;
  }

  apc_store('foo_same', $value);

  $value = vec[];
  for ($i = 0; $i < 100000; $i++) {
    $value[] = 'stringstringstringstringstring1';
  }

  apc_store('foo_static', $value);

  $info = apc_cache_info();

  $memory_usage = dict[];
  foreach ($info['cache_list'] as $entry) {
    $memory_usage[$entry['info']] = $entry['mem_size'];
  }

  echo 'different:'."\n";
  var_dump($memory_usage['foo_static'] * 10 < $memory_usage['foo_different']);
  echo 'unique:'."\n";
  var_dump($memory_usage['foo_static'] * 0.9 < $memory_usage['foo_unique']);
  var_dump($memory_usage['foo_static'] * 1.1 > $memory_usage['foo_unique']);
  echo 'same:'."\n";
  var_dump($memory_usage['foo_static'] * 0.9 < $memory_usage['foo_same']);
  var_dump($memory_usage['foo_static'] * 1.1 > $memory_usage['foo_same']);
}
