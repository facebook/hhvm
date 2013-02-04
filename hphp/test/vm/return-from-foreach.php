<?php
function f1() {
  $arr = array(1,2,3);
  foreach ($arr as $v1) {
    foreach ($arr as $v2) {
      return;
    }
  }
}
function f2() {
  $arr = array(1,2,3);
  foreach ($arr as &$v1) {
    foreach ($arr as $v2) {
      return;
    }
  }
}
function f3() {
  $arr = array(1,2,3);
  foreach ($arr as $v1) {
    foreach ($arr as &$v2) {
      return;
    }
  }
}
function f4() {
  $arr = array(1,2,3);
  foreach ($arr as &$v1) {
    foreach ($arr as &$v2) {
      return;
    }
  }
}
f1();
f2();
f3();
f4();
echo "Done\n";


