<?hh

const int TEST_TTL = 3;

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
      // There's a race condition here - APC TTLs are based on second-resolution
      // accuracy so if we set a TTL of 1s and happen to cross the second
      // boundary before the fetch (unlikely * runs a lot == it happens) it will
      // have already timed out. Since we're running 10 times with a delay of 1s
      // then a 3s TTL should be plenty.
      $x = apc_add($prefix.'add', makestring('add'), TEST_TTL);
      $results[] = $prefix.'add: '.apc_fetch_null($prefix.'add');
      apc_add($prefix.'add', makestring('add'), TEST_TTL);
      $results[] = $prefix.'add: '.apc_fetch_null($prefix.'add');
      apc_store($prefix.'store', makestring('store'), TEST_TTL);
      $results[] = $prefix.'store: '.apc_fetch_null($prefix.'store');
      apc_store($prefix.'store', makestring('store'), TEST_TTL);
      $results[] = $prefix.'store: '.apc_fetch_null($prefix.'store');
      apc_store($prefix.'extend', makestring('extend'), TEST_TTL);
      $results[] = $prefix.'extend: '.apc_fetch_null($prefix.'extend');
      apc_extend_ttl($prefix.'extend', TEST_TTL);
      $results[] = $prefix.'extend: '.apc_fetch_null($prefix.'extend');
      // Sleep for 1s
      await SleepWaitHandle::create(1000000);
      break;
    case 9:
      $results[] = $prefix.'add: '.(string)(apc_fetch_null($prefix.'add'));
      $results[] = $prefix.'store: '.(string)(apc_fetch_null($prefix.'store'));
      $results[] = $prefix.'extend: '.(string)(apc_fetch_null($prefix.'extend'));
      break;
    default:
      if ($times < 9) {
        // Sleep for 1s
        await SleepWaitHandle::create(1000000);
      } else {
        throw new Exception('Test ran too many times: '.$times);
      }
      break;
  }

  return $results;
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  // The test runner runs this test 10 times (because of the opts file) - the
  // first iteration sets up APC. Middle iterations just sleep. The final
  // iteration checks that APC values have expired.
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
