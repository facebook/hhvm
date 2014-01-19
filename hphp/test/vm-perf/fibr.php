<?php

# Compute the n'th Fibonacci number recursively, where n >= 0.
function fib($n) {
  if ($n > 1) {
    return fib($n-2) + fib($n-1);
  } else {
    return $n;
  }
}

function tail_recurse($n) {
  if ($n > 0) {
    tail_recurse($n-1);
  }
  print $n; print ": "; print fib($n); print "\n";
}

tail_recurse(35);
