<?hh

define('PREFIX', 'HPHP_TEST_MCROUTER|');

async function set_get_del(MCRouter $mcr): Awaitable<string> {
  $key = PREFIX . 'set-get-del';
  $in = 'banana';
  await $mcr->set($key, $in);
  $out = await $mcr->get($key);
  if ($in !== $out) {
    return "Got different value '$out' than stored '$in'";
  }
  await $mcr->del($key);
  try {
    await $mcr->get($key);
    throw new Exception("Got value where none was expected");
  } catch (McRouterException $e) {
    // Expected
  }

  return "Success";
}

async function set_inc_dec(MCRouter $mcr): Awaitable<string> {
  $key = PREFIX . 'set-inc-dec';
  await $mcr->set($key, 42);
  $newval = await $mcr->incr($key, 8);
  if ($newval !== 50) {
    return "Failed to increment, exp 50, got $newval";
  }
  $newval = await $mcr->decr($key, 25);
  if ($newval !== 25) {
    return "Failed to decrement, exp 25, got $newval";
  }
  await $mcr->del($key);

  return "Success";
}

$servers = Vector { getenv('HPHP_TEST_MCROUTER') };
$mcr = MCRouter::createSimple($servers);
$wh = Vector {
  set_get_del($mcr),
  set_inc_dec($mcr),
};
$results = HH\Asio\join(HH\Asio\v($wh));
foreach ($results as $result) {
  var_dump($result);
}
