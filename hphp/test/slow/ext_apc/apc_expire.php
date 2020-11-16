<?hh

function apc_fetch_null<T>(string $key): ?T {
  $success = false;
  $ret = apc_fetch($key, inout $success);
  return $success ? $ret : null;
}

function makestring(string $str): string {
  return $str.__hhvm_intrinsics\launder_value('val');
}

async function test(string $prefix, int $times): Awaitable<vec<(string, string)>> {
  $results = vec[];

  switch ($times) {
    case 0:
      apc_add($prefix.'add', makestring('add'), 1);
      $results[] = $prefix.'add: '.apc_fetch_null($prefix.'add');
      apc_add($prefix.'add', makestring('add'), 3);
      $results[] = $prefix.'add: '.apc_fetch_null($prefix.'add');
      apc_store($prefix.'store', makestring('store'), 1);
      $results[] = $prefix.'store: '.apc_fetch_null($prefix.'store');
      apc_store($prefix.'store', makestring('store'), 3);
      $results[] = $prefix.'store: '.apc_fetch_null($prefix.'store');
      apc_store($prefix.'extend', makestring('extend'), 1);
      $results[] = $prefix.'extend: '.apc_fetch_null($prefix.'extend');
      apc_extend_ttl($prefix.'extend', 3);
      $results[] = $prefix.'extend: '.apc_fetch_null($prefix.'extend');
      await SleepWaitHandle::create(1000000);
      break;
    case 9:
      $results[] = $prefix.'add: '.apc_fetch_null($prefix.'add');
      $results[] = $prefix.'store: '.apc_fetch_null($prefix.'store');
      $results[] = $prefix.'extend: '.apc_fetch_null($prefix.'extend');
      break;
    default:
      if ($times < 9) {
        await SleepWaitHandle::create(1000000);
      } else {
        throw new Exception('To big times '.$times);
      }
      break;
  }

  return $results;
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  $success = false;
  $count = apc_fetch('times', inout $success) ?: 0;
  concurrent {
    $nothot_results = await test('nothot:', $count);
    $hot_results = await test('hothot:', $count);
  }

  if ($nothot_results || $hot_results) {
    var_dump($nothot_results);
    var_dump($hot_results);
  }

  apc_store('times', $count + 1);
}
