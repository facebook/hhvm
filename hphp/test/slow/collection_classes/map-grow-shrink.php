<?php
function main() {
  $m = Map {};
  $m[1] = 1;
  unset($m[1]);
  $m[1] = 1;
  $m[2] = 2;
  unset($m[2]);
  unset($m[1]);
  $m[1] = 1;
  $m[2] = 2;
  $m[3] = 3;
  unset($m[3]);
  unset($m[2]);
  unset($m[1]);
  for ($i = 1; $i <= 100; ++$i) $m[$i] = $i;
  for ($i = 100; $i >= 1; --$i) unset($m[$i]);
  for ($i = 1; $i <= 10000; ++$i) $m[$i] = $i;
  for ($i = 10000; $i >= 1; --$i) unset($m[$i]);
  echo "Done\n";
}
main();
