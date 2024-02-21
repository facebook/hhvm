<?hh

function do_a_parse($f) {
  HH\FileDecls::parsePath($f);
}
<<__EntryPoint>>
function main() {
  // Get the autoloader to initialize
  HH\autoload_is_native();
  if (HH\execution_context() === 'xbox') return;
  $r = vec[];
  echo "starting\n";
  $tmpdir = sys_get_temp_dir();
    for ($i = 0; $i < 500; $i++) {
    $f = "$tmpdir/foo_$i.php";
    file_put_contents($f, "<?hh class Foo {}\nclass Bar extends Foo {}");
    $r[] = fb_call_user_func_async(__FILE__, 'do_a_parse', $f);
  }
  $start = new DateTime();
  foreach ($r as $xbox) {
    fb_end_user_func_async($xbox);
  }
  $end = new DateTime();
  // This is ideally to catch either deadlocks (test will hang) or
  // perf regressions (taking forever to parse files due to lock contention).
  echo "took less than 5 seconds? ". ($end->diff($start)->s < 5 ? 'yes' : 'no')."\n";
  echo "finished all runs\n";
}
