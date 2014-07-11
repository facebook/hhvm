<?php
$f = fopen('/dev/zero', 'r');
fread($f, 1<<32);
print "Done\n";
