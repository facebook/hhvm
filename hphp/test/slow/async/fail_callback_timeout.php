<?hh

function a() {
  while (true) { mt_rand(); }
}

async function heh() {
  await rescheduleWaitHandle::Create(0, 0);
  throw new exception('x');
}

set_time_limit(6);
AsyncFunctionWaitHandle::setOnFailCallback(() ==> a());
$x = heh();
try {
  HH\Asio\join($x);
} catch (exception $y) {
}
while (true) { mt_rand(); }
