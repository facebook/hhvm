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
      apc_store($prefix.'foofoo', makestring('foo'), 16, 8);
      $results[] = $prefix.'foofoo: '.apc_fetch_null($prefix.'foofoo');
      apc_store($prefix.'barbar', makestring('bar'), 16, 8);
      $results[] = $prefix.'barbar: '.apc_fetch_null($prefix.'barbar');
      apc_store($prefix.'bazbaz', makestring('baz'), 16, 8);
      $results[] = $prefix.'bazbaz: '.apc_fetch_null($prefix.'bazbaz');
      await SleepWaitHandle::create(2000000);
      break;
    case 1:
      $results[] = $prefix.'bazbaz: '.apc_fetch_null($prefix.'bazbaz');
      await SleepWaitHandle::create(4000000);
      break;
    case 2:
      $results[] = $prefix.'foofoo: '.apc_fetch_null($prefix.'foofoo');
      await SleepWaitHandle::create(3000000);
      break;
    case 3:
      $results[] = $prefix.'bazbaz: '.apc_fetch_null($prefix.'bazbaz');
      await SleepWaitHandle::create(3000000);
      break;
    case 4:
      $results[] = $prefix.'foofoo: '.apc_fetch_null($prefix.'foofoo');
      $results[] = $prefix.'barbar: '.apc_fetch_null($prefix.'barbar');
      break;
    default:
      throw new Exception('To big times '.$times);
  }

  return $results;
}

<<__EntryPoint>>
async function main(): Awaitable<void> {
  $count = apc_fetch_null('times') ?? 0;
  concurrent {
    $nothot_results = await test('nothot:', $count);
    $hot_results = await test('hot:', $count);
  }

  var_dump($nothot_results);
  var_dump($hot_results);

  apc_store('times', $count + 1);
}
