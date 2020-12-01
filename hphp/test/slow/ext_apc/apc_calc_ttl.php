<?hh

<<__EntryPoint>>
function main(): void {
  $max_ttls = vec[-1, 0, 10, 100000000000];
  $bump_ttls = vec[-1, 0, 5, 10, 20, 100000000000, 100000000001];

  for ($i = 0; $i < 10; $i++) {
    $start_time = time();
    foreach ($max_ttls as $max_ttl) {
      foreach ($bump_ttls as $bump_ttl) {
        apc_store('foo_'.$max_ttl.'_'.$bump_ttl, 'value', $max_ttl, $bump_ttl);
      }
    }

    $cache_list = apc_cache_info()['cache_list'];
    if ($start_time === time()) {
      $data = dict[];
      foreach ($cache_list as $c) {
        $data[$c['info']] = tuple($c['ttl'], $c['max_ttl'], $c['bump_ttl']);
      }
      ksort(inout $data);
      var_dump($data);
      return;
    }
  }
  throw new Exception('Things went to slow!');
}
