<?php

class X {
  function __call($asd, ...$ry) {
    echo "ok\n";
    var_dump($ry);
  }
}


(new X)->__call(new stdclass, new stdclass, new stdclass);
