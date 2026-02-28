<?hh

async function test0($x) :Awaitable<mixed>{
  await SleepWaitHandle::create(1000);
  if ($x % 3 === 0) throw new Exception('thrown');
  return $x;
}

async function test1($x) :Awaitable<mixed>{
  return await test0($x + 1);
}

async function test2($x) :Awaitable<mixed>{
  return await test1($x + 1);
}

async function test3($x) :Awaitable<mixed>{
  return await test2($x + 1);
}

async function test4($x) :Awaitable<mixed>{
  return await test3($x + 1);
}

async function test5($x) :Awaitable<mixed>{
  return await test4($x + 1);
}

async function test6($x) :Awaitable<mixed>{
  return await test5($x + 1);
}

async function test7($x): Awaitable<int> {
  return await test6($x + 1);
}

async function test8($x) :Awaitable<mixed>{
  return await test7($x + 1);
}

async function test9($x) :Awaitable<mixed>{
  try {
    return await test8($x + 1);
  } catch (Exception $e) {
    return 'caught';
  }
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  for ($i = 0; $i < 3; $i++) {
    print("\n======================\nIteration $i:\n");
    $results = vec[
      test0($i),
      test1($i),
      test2($i),
      test3($i),
      test4($i),
      test5($i),
      test6($i),
      test7($i),
      test8($i),
      test9($i),
    ];
    foreach ($results as $result) {
      try {
        $result = await $result;
        var_dump($result);
      } catch (Exception $e) {
        var_dump($e->toString());
      }
    }
  }
}
