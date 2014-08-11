<?php
function fact($x) {
  if($x <= 1) {
    return 1;
  }

  return gmp_mul($x, fact($x-1));
}

print gmp_strval(fact(1000)) . "\n";
