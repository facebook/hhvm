<?hh

<<__Memoize>>
async function a() :Awaitable<mixed>{
  // make sure we suspend
  await RescheduleWaitHandle::create(0, 0);

  // uses async eager return
  await b();
}

async function b() :Awaitable<mixed>{
  await c();
}

async function c() :Awaitable<mixed>{
  await d();
}

async function d() :Awaitable<mixed>{
  // does not use async eager return
  await a();
}


<<__EntryPoint>>
async function main_eager_return_interp() :Awaitable<mixed>{
  try {
    await a();
  } catch (Exception $e) {
    echo "Caught!\n";
  }
}
