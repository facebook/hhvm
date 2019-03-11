<?hh
function throw_one_time($why, $what) {

  if ($what === 'a' && $why == 'exit') {
    if (AsyncSuspendHookThrow::$counter++ == 1) {
      echo "throwing\n";
      throw new exception('x');
    }
  }
}

async function a() {
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "a woke up\n";
  await RescheduleWaitHandle::create(RescheduleWaitHandle::QUEUE_DEFAULT,0);
  echo "a woke up2\n";
}

async function d() {

  try { await AsyncSuspendHookThrow::$z; } catch (Exception $x) { echo "d_catch\n"; }
  echo "heyo d\n";
}

async function c() {

  try { await AsyncSuspendHookThrow::$z; } catch (Exception $x) { echo "c_catch\n"; }
  echo "c woke up\n";
}


<<__EntryPoint>>
function main_suspend_hook_throw() {
$counter = 0;

fb_setprofile('throw_one_time');

AsyncSuspendHookThrow::$z = a();
$l = d();

$k = c();
HH\Asio\join($l);
HH\Asio\join($k);
}

abstract final class AsyncSuspendHookThrow {
  public static $counter;
  public static $z;
}
