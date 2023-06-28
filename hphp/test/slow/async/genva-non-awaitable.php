<?hh

class Foo {}

async function test_short($input) :Awaitable<mixed>{
  var_dump($input);
  try {
    concurrent {
      await async {};
      await $input;
      await async {};
    }
    echo "ok\n";
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}

async function test_long($input) :Awaitable<mixed>{
  var_dump($input);
  try {
    concurrent {
      await async {};
      await async {};
      await async {};
      await async {};
      await $input;
      await async {};
      await async {};
      await async {};
      await async {};
    }
    echo "ok\n";
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}

async function test() :Awaitable<mixed>{
  await test_short(null);
  await test_short(async {});
  await test_short(RescheduleWaitHandle::create(0, 0));
  await test_short(0);
  await test_short(true);
  await test_short('hello world');
  await test_short(dict[]);
  await test_short(new Foo());
  await test_long(null);
  await test_long(async {});
  await test_long(RescheduleWaitHandle::create(0, 0));
  await test_long(0);
  await test_long(true);
  await test_long('hello world');
  await test_long(dict[]);
  await test_long(new Foo());
}


<<__EntryPoint>>
function main_genva_non_awaitable() :mixed{
HH\Asio\join(test());
}
