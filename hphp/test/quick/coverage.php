<?hh

function b($s) :mixed{
  if ($s === 'floo') {
    return 12;
  }
  \HH\global_set('z', 234);
  return 8;
}

function a($b) :mixed{
  if ($b) {
    $x = 'floo';
  } else {
    $x = 'fleeee';
  }
  b($x);
}

function f() :mixed{
  for ($i = 0; $i < 4; ++$i) {
    a($i < 3);
    \HH\global_set('tmp', $i);
    $i = \HH\global_get('tmp');
  }
}

function enable() :mixed{
  echo "Going to enable\n";
  fb_enable_code_coverage();
  echo "Enabled\n";
  \HH\global_set('z', 3);
}

function doenable() :mixed{

  enable();
  Coverage::$y += 43;
}

function main() :mixed{
  echo "About to enable\n";
  doenable();
  echo "Done enabling\n";
  f();
  $r = fb_disable_code_coverage();
  unset($r['/:ext_fb.php']);
  var_dump($r);
}
abstract final class Coverage {
  public static $y = 0;
}
<<__EntryPoint>>
function main_entry(): void {
  main();
}
