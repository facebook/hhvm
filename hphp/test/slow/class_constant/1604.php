<?php

class parent_c {
  const ZERO   = 0;
  const TWENTY = 20;
  const FORTY  = 40;
}
class child_c extends parent_c {
  const FIFTY = 50;
}
function foo($a = parent_c::ZERO, $b = child_c::FIFTY, $c = child_c::FORTY) {
  echo $a;
  echo $b;
  echo $c;
}
foo();
print parent_c::ZERO;
