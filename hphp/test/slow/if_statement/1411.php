<?php

if (false) {
  echo "case 1\n";
}
if (false) {
  echo "case 2\n";
  function f1() {
    $v = 1;
  }
}
if (true) {
  echo "case 3\n";
}
if (true) {
  echo "case 4\n";
  function f2() {
    $v = 1;
  }
}
if (true) {
  echo "case 5\n";
}
 else {
  echo "case 6\n";
}
if (true) {
  echo "case 7\n";
}
 else {
  echo "case 8\n";
  function f3() {
    $v = 1;
  }
}
if ($a) {
  echo "case 9\n";
}
 elseif (true) {
  echo "case 10\n";
  function f4() {
    $v = 1;
  }
}
 else {
  echo "case 11\n";
}
if ($a) {
  echo "case 12\n";
}
 elseif (true) {
  echo "case 13\n";
}
 else {
  echo "case 14\n";
  function f5() {
    $v = 1;
  }
}
if ($a) {
  echo "case 15\n";
}
 elseif (true) {
  echo "case 16\n";
}
 elseif (true) {
  echo "case 17\n";
  function f6() {
    $v = 1;
  }
}
 else {
  echo "case 18\n";
}
if ($a) {
  echo "case 19\n";
}
 elseif (true) {
  echo "case 20\n";
}
 elseif (true) {
  echo "case 21\n";
  function f7() {
    $v = 1;
  }
}
if ($a) {
  echo "case 22\n";
}
 elseif (true) {
  echo "case 23\n";
}
 elseif (false) {
  echo "case 24\n";
  function f8() {
    $v = 1;
  }
}
 else {
  echo "case 25\n";
}
if ($a) {
  echo "case 26\n";
}
 elseif (true) {
  echo "case 27\n";
}
 elseif (false) {
  echo "case 28\n";
  function f9() {
    $v = 1;
  }
}
