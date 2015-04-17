<?php

class __call {
  function __call($x, $y) { echo "ok\n"; var_dump($x, $y); }
}
new __call(1,2);
