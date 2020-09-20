<?hh

<<__Memoize>>
async function a() {
  // make sure we suspend
  await RescheduleWaitHandle::create(0, 0);

  // uses async eager return
  await b();
}

async function b() {
  await c();
}

async function c() {
  await d();
}

async function d() {
  // does not use async eager return
  await a();
}


<<__EntryPoint>>
async function main_eager_return_interp() {
  try {
    await a();
  } catch (Exception $e) {
    echo "Caught!\n";
  }
}
