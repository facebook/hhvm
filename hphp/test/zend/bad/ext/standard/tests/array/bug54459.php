<?php
foreach (range(90, 100, .1) as $i => $v){
  echo $i, ' = ', $v, PHP_EOL;
}
foreach (range("90", "100", .1) as $i => $v){
  echo $i, ' = ', $v, PHP_EOL;
}