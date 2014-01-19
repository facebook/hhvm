<?php

class a extends Exception {
}
;
class b extends a {
  function dump() {
    echo 'c:', $this->code, '
m:', $this->message, '
';
    echo 'x:', $this->x, '
y:', $this->y, '
';
  }
}
if (0) {
 class a extends Exception {
}
 }
try {
  throw(new b(1, 2));
}
 catch (b $e) {
  $e->dump();
}
