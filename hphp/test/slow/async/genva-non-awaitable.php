<?hh

class Foo {}

async function test_short($input) {
  var_dump($input);
  try {
    await genva(
      async {},
      $input,
      async {},
    );
    echo "ok\n";
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}

async function test_long($input) {
  var_dump($input);
  try {
    await genva(
      async {},
      async {},
      async {},
      async {},
      $input,
      async {},
      async {},
      async {},
      async {},
    );
    echo "ok\n";
  } catch (Exception $e) {
    echo $e->getMessage()."\n";
  }
}

async function test() {
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
function main_genva_non_awaitable() {
HH\Asio\join(test());
}
