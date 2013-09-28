<?php

define('AAA', true);
define('BBB', false);
define('CCC', null);
if (AAA){
  echo "AAA";
}
 else {
  echo "!AAA";
}
if (BBB) {
  echo "BBB";
}
 else {
  echo "!BBB";
}
if (CCC) {
  echo "CCC";
}
 else {
  echo "!CCC";
}
$a = AAA ? "AAA" : "!AAA";
$b = BBB ? "BBB" : "!BBB";
$c = CCC ? "CCC" : "!CCC";
echo "$a$b$c\n";
