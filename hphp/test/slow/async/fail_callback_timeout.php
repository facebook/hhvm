<?hh

function a() {
  while (true) { mt_rand(); }
}

async function heh() {
  await RescheduleWaitHandle::create(0, 0);
  throw new Exception('x');
}
<<__EntryPoint>> function main(): void {
set_time_limit(6);
AsyncFunctionWaitHandle::setOnFailCallback(() ==> a());
$x = heh();
try {
  HH\Asio\join($x);
} catch (Exception $y) {
}
while (true) { mt_rand(); }
}
