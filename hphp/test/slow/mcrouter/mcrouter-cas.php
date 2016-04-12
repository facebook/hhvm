<?hh

async function set_get_cas_get_del(MCRouter $mcr, $fail): Awaitable<string> {
  $key = 'HPHP_TEST_MCROUTER_CAS|set_get_cas_get_del|'.
         ($fail ? 'fail' : 'succeed');

  await $mcr->set($key, 'banana');
  $record = await $mcr->gets($key);
  if (!isset($record['value']) ||
      ($record['value'] !== 'banana')) {
    return "Failed retrieving initial set value: " . print_r($record, true);
  }

  $cas = $record['cas'];
  if ($fail) ++$cas;
  try {
    await $mcr->cas($cas, $key, 'orange');
  } catch (MCRouterException $e) {
    return "Failed: " . $e->getMessage();
  }

  $val = await $mcr->get($key);
  if ($val !== 'orange') {
    return "Failed performing CAS for $key => $val";
  }
  await $mcr->del($key);
  return "Success";
}

$servers = Vector { getenv('HPHP_TEST_MCROUTER') };
$mcr = MCRouter::createSimple($servers);
$wh = Vector {
  set_get_cas_get_del($mcr, false),
  set_get_cas_get_del($mcr, true),
};
$results = HH\Asio\join(HH\Asio\v($wh));
foreach ($results as $result) {
  var_dump($result);
}
