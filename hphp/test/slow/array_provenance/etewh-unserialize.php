<?hh

async function gen_sleep() {
  await SleepWaitHandle::create(1000);
}

async function gen_stuff() {
  await gen_sleep();

  $wh = __hhvm_intrinsics\dummy_dict_await();
  $d = await $wh;

  var_dump($d);
  return $d;
}

<<__EntryPoint>>
function main() {
  $wh = gen_stuff();
  $d = HH\Asio\join($wh);
  var_dump(HH\get_provenance($d));
}
