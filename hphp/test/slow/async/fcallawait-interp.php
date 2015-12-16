<?hh

<<__Memoize>>
async function a() {
  // make sure we suspend
  await RescheduleWaitHandle::create(0, 0);

  // FCallAwait
  await b();
}

async function b() {
  await c();
}

async function c() {
  await d();
}

async function d() {
  // not a FCallAwait
  await a();
}

try {
  a()->join();
} catch (Exception $e) {
  echo "Caught!\n";
}
