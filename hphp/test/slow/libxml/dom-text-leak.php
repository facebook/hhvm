<?php
// c.f. https://github.com/facebook/hhvm/issues/4084

function foo() {
  for ($i=0; $i<100; $i++) {
    new DOMText('hi');
  }
}

foo(); foo(); foo();
$start = memory_get_peak_usage(true);
foo();
$end = memory_get_peak_usage(true);
echo $end - $start, "\n";
