<?php

print "Test begin\n";

# Get a key that doesn't exist in $GLOBALS.
$k = 'k';
while (isset($GLOBALS[$k])) {
  $k .= 'k';
}

$a = $GLOBALS;
$GLOBALS[$k] = 42;
if (isset($GLOBALS[$k])) {
  print "Key is set in \$GLOBALS\n";
}
if (isset($a[$k])) {
  print "Key set visible via \$a\n";
}
$b = $a;
$a[$k]++;
if ($b[$k] == 43) {
  print "Key update visible via \$b\n";
}

print "Test end\n";
