<?hh

function b($s) {
  if ($s === 'floo') {
    return 12;
  }
  \HH\global_set('z', 234);
  return 8;
}

function a($b) {
  if ($b) {
    $x = 'floo';
  } else {
    $x = 'fleeee';
  }
  b($x);
}

function f() {
  for ($i = 0; $i < 4; ++$i) {
    a($i < 3);
    \HH\global_set('tmp', $i);
    $i = \HH\global_get('tmp');
  }
}

function enable() {
  echo "Going to enable\n";
  fb_enable_code_coverage();
  echo "Enabled\n";
  \HH\global_set('z', 3);
}

function doenable() {

  enable();
  Coverage::$y += 43;
}

function main() {
  echo "About to enable\n";
  doenable();
  echo "Done enabling\n";
  f();
  $r = fb_disable_code_coverage();
  unset($r['/:systemlib.phpfb']);
  var_dump($r);
}
abstract final class Coverage {
  public static $y;
}
<<__EntryPoint>>
function main_entry(): void {
  main();
}
