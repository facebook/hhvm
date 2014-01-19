<?php

for ($i = 0;
$i<3;
$i++) {
  echo "Start Of I loop\n";
  for ($j=0;
;
$j++) {
    if ($j >= 2) continue 2;
      echo "I : $i J : $j"."\n";
  }
  echo "End\n";
}
for ($i = 0;
$i<10;
$i++) {
  if ($i % 2 == 0) continue 1;
  echo $i . "\n";
}
for ($i = 0;
$i<10;
$i++) {
  if ($i % 2 == 0) continue;
  echo $i . "\n";
}
