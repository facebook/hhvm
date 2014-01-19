<?php
$h = new SplMaxHeap();
$h->insert(1);

function myfunc($a, $b) {
  error_log($a.$b);
}

class MyClass {
  function myMeth($a, $b) {
    error_log($a.$b);
  }
}
