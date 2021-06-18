<?hh

function doh($x, $y) {
  if ($y === 'hey') {
    if ($x === 'exit') {

      if ((YieldYieldSuspendHook::$counter++ % 2) == 1) {
        throw new exception('x');
      }
    }
  }
}

function hey() {
  yield new stdClass;
}


<<__EntryPoint>>
function main_yield_suspend_hook() {
fb_setprofile('doh');

for ($i = 0; $i < 3; ++$i) {
  try { foreach (hey() as $k) {} } catch (Exception $x) {}
}
}

abstract final class YieldYieldSuspendHook {
  public static $counter;
}
