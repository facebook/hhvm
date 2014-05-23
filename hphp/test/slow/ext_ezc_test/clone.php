<?php
$a = new EzcTestCloneable;
$b = clone $a;
$c = clone $b;
print "Deleting a\n";
$a = null;
print "Deleting b\n";
$b = null;
print "Deleting c\n";
$c = null;
print "Done\n";
