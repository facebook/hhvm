<?php

function main() {
  $arr = null;
  print "start iter loop\n";
  foreach ($arr as $x => $y) {
    print "fail";
  }
  print "end iter loop\n";
  $ref = &$arr;
  print "start witer loop\n";
  foreach ($arr as $x => $y) {
    print "fail";
  }
  print "end witer loop\n";
  print "start miter loop\n";
  foreach ($arr as $x => &$y) {
    print "fail";
  }
  print "end of miter loop\n";
}

main();

