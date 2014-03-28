<?php

$base_dir = __DIR__;

$temp = $base_dir . '/touch-temp-test.php';
@unlink($temp);

function run_trivial_file($filename, $ret) {
  file_put_contents($filename, "<?php return $ret;");

  // stupid, but sometimes stat() isn't enough to notice changes
  // in modified files
  usleep(5000);

  return (include $filename);
}

for ($i = 0; $i < 2; ++$i) {
  $res1 = run_trivial_file($temp, 1);
  $res2 = run_trivial_file($temp, 1);

  foreach (array($res1, $res2) as $res) {
    if ($res !== 1) {
      echo "What happened!? Return was '$res' instead of '1'\n";
      exit(1);
    }
  }
}

@unlink($temp);
