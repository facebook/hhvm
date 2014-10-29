<?php

function main() {
  $a = array(0, 1, 2);
  print ":".empty($a[-1]).":\n";
  print ":".empty($a[0]).":\n";
  print ":".empty($a[1]).":\n";
  print ":".empty($a[3]).":\n";
  print ":".empty($a["blah"]).":\n";
  print "\n";

  $a = "";
  print ":".empty($a[-1]).":\n";
  print ":".empty($a[0]).":\n";
  print ":".empty($a[1]).":\n";
  print ":".empty($a["blah"]).":\n";
  print ":".empty($a[0][0]).":\n";
  print "\n";

  $a = "a01";
  print ":".empty($a[-1]).":\n";
  print ":".empty($a[0]).":\n";
  print ":".empty($a[1]).":\n";
  print ":".empty($a[2]).":\n";
  print ":".empty($a[3]).":\n";
  print ":".empty($a["blah"]).":\n";
  print ":".empty($a[0][0]).":\n";
  print "\n";

  $a = 0;
  print ":".empty($a[0]).":\n";
  print "\n";

  $a = 42;
  print ":".empty($a[0]).":\n";
  print "\n";

  $a = false;
  print ":".empty($a[0]).":\n";
  print "\n";

  $a = 2.0;
  print ":".empty($a[0]).":\n";
  print "\n";
}
main();
