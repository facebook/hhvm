<?php
# Compute the n'th Fibonacci number recursively, where n >= 0.
function fib($n) {
  if ($n > 1) {
    return fib($n-2) + fib($n-1);
  } else {
    return $n;
  }
}

print " 0: "; print fib(0); print "\n";
print " 1: "; print fib(1); print "\n";
print " 2: "; print fib(2); print "\n";
print " 3: "; print fib(3); print "\n";
print " 4: "; print fib(4); print "\n";
print " 5: "; print fib(5); print "\n";
print " 6: "; print fib(6); print "\n";
print " 7: "; print fib(7); print "\n";
print " 8: "; print fib(8); print "\n";
print " 9: "; print fib(9); print "\n";

print "10: "; print fib(10); print "\n";
print "11: "; print fib(11); print "\n";
print "12: "; print fib(12); print "\n";
print "13: "; print fib(13); print "\n";
print "14: "; print fib(14); print "\n";
print "15: "; print fib(15); print "\n";
print "16: "; print fib(16); print "\n";
print "17: "; print fib(17); print "\n";
print "18: "; print fib(18); print "\n";
print "19: "; print fib(19); print "\n";

print "20: "; print fib(20); print "\n";
print "21: "; print fib(21); print "\n";
print "22: "; print fib(22); print "\n";
print "23: "; print fib(23); print "\n";
print "24: "; print fib(24); print "\n";
print "25: "; print fib(25); print "\n";
print "26: "; print fib(26); print "\n";
print "27: "; print fib(27); print "\n";
print "28: "; print fib(28); print "\n";
print "29: "; print fib(29); print "\n";

print "30: "; print fib(30); print "\n";
print "31: "; print fib(31); print "\n";
print "32: "; print fib(32); print "\n";
print "33: "; print fib(33); print "\n";
print "34: "; print fib(34); print "\n";
print "35: "; print fib(35); print "\n";
