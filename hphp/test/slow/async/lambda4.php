<?hh

async function block() {
  await RescheduleWaitHandle::create(0, 0);
}

async function foo($a) {
  $fn = async ($b, $c) ==> {
    await block();
    return $a * $b + $c;
  };

  return await $fn(47, 26);
}

var_dump(foo(42)->join());
