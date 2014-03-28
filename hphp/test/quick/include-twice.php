<?php

$base_dir = __DIR__;

$temp = $base_dir . '/include-twice-test.php';
@unlink($temp);

function run_trivial_file($filename, $ret) {
  file_put_contents($filename, "<?php return $ret;");

  // stupid, but sometimes stat() isn't enough to notice changes
  // in modified files
  usleep(5000);

  return (include $filename);
}

for ($i = 0; $i < 20; ++$i) {
  $res1 = run_trivial_file($temp, 1);
  $res2 = run_trivial_file($temp, 2);

  if ($res1 !== 1) {
    echo "Bad include, got '$res1' instead of 1\n";
  }
  if ($res2 !== 2) {
    echo "Bad include, got '$res2' instead of 2\n";
  }
}

@unlink($temp);
