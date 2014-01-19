<?php

$a = array(0, 1, 2);
print ":".isset($a[-1]).":\n";
print ":".isset($a[0]).":\n";
print ":".isset($a[1]).":\n";
print ":".isset($a[3]).":\n";
print ":".isset($a["blah"]).":\n";
print "\n";

$a = array(null, null);
print ":".isset($a[-1]).":\n";
print ":".isset($a[0]).":\n";
print ":".isset($a[1]).":\n";
print ":".isset($a[2]).":\n";
print "\n";

$a = "";
print ":".isset($a[-1]).":\n";
print ":".isset($a[0]).":\n";
print ":".isset($a[1]).":\n";
print ":".isset($a["blah"]).":\n";
print ":".isset($a[0][0]).":\n";
print "\n";

$a = "a01";
print ":".isset($a[-1]).":\n";
print ":".isset($a[0]).":\n";
print ":".isset($a[1]).":\n";
print ":".isset($a[2]).":\n";
print ":".isset($a[3]).":\n";
print ":".isset($a["blah"]).":\n";
print ":".isset($a[0][0]).":\n";
print "\n";

$a = 0;
print ":".isset($a[0]).":\n";
print "\n";

$a = 42;
print ":".isset($a[0]).":\n";
print "\n";

$a = false;
print ":".isset($a[0]).":\n";
print "\n";

$a = 2.0;
print ":".isset($a[0]).":\n";
print "\n";
