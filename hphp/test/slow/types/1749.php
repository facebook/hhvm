<?php

class X {
}
;
function bug() {
  if (!$GLOBALS['x']) {
    return;
  }
  return new X;
}
var_dump(bug());
