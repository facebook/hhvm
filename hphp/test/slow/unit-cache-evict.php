<?hh

<<__EntryPoint>>
function main() {
  $iter = (int)apc_fetch('iter_id');
  apc_store('iter_id', $iter + 1);

  if ($iter === 0) {
    $file = tempnam(sys_get_temp_dir(), 'unit-cache-target-');
    $cachebreaker = mt_rand().' '.time();
    apc_store('cachebreaker', $cachebreaker);
    apc_store('tmpfile', $file);
  } else {
    $cachebreaker = apc_fetch('cachebreaker');
    $file = apc_fetch('tmpfile');
  }

  $last_line = $iter > 3 ? "// iter: $iter" : "// empty";
  $contents = <<<EOT
<?hh // $cachebreaker

function hello(\$x, \$y) { return \$x + \$y; }
$last_line
EOT;

  sleep(1);
  file_put_contents($file, $contents);
  require($file);
  if (hello(1, 2) !== 3) echo "ERROR!\n";

  $loaded = implode(hh\get_compiled_units(1), ', ');
  $compiled = implode(hh\get_compiled_units(0), ', ');
  $evicted  = implode(hh\get_compiled_units(-1), ', ');

  if ($iter !== 0) {
    echo "Iter $iter: LOADED($loaded) COMPILED($compiled) EVICTED($evicted)\n";
  }

  if ($iter === 6) unlink($file);
}
