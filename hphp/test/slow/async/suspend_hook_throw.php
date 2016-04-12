<?hh

$counter = 0;
function throw_one_time($why, $what) {
  global $counter;
  if ($what === 'a' && $why == 'exit') {
    if ($counter++ == 1) {
      echo "throwing\n";
      throw new exception('x');
    }
  }
}

fb_setprofile('throw_one_time');

async function a() {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "a woke up\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "a woke up2\n";
}

global $z;
$z = a();

async function d() {
  global $z;
  try { await $z; } catch (Exception $x) { echo "d_catch\n"; }
  echo "heyo d\n";
}
$l = d();

async function c() {
  global $z;
  try { await $z; } catch (Exception $x) { echo "c_catch\n"; }
  echo "c woke up\n";
}

$k = c();
HH\Asio\join($l);
HH\Asio\join($k);
