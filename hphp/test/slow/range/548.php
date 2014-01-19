<?php

foreach (range("0 xxx", "12 yyy") as $number) {
  echo $number . "
";
}
foreach (range("0", "12") as $number) {
  echo $number . "
";
}
foreach (range('a', 'i') as $letter) {
  echo $letter;
}
foreach (range('c', 'a') as $letter) {
  echo $letter;
}
foreach (range('a xxx', 'i yyy') as $letter) {
  echo $letter;
}
foreach (range('c xxx', 'a yyy') as $letter) {
  echo $letter;
}
