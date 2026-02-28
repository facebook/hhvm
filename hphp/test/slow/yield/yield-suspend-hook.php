<?hh

function doh($x, $y) :mixed{
  if ($y === 'hey') {
    if ($x === 'exit') {

      if ((YieldYieldSuspendHook::$counter++ % 2) == 1) {
        throw new Exception('x');
      }
    }
  }
}

function hey() :AsyncGenerator<mixed,mixed,void>{
  yield new stdClass;
}


<<__EntryPoint>>
function main_yield_suspend_hook() :mixed{
fb_setprofile('doh');

for ($i = 0; $i < 3; ++$i) {
  try { foreach (hey() as $k) {} } catch (Exception $x) {}
}
}

abstract final class YieldYieldSuspendHook {
  public static $counter = 0;
}
