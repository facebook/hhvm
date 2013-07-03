<?php

namespace foo {
  function bar($a, $b) {
    var_dump(func_num_args(), func_get_args());
  }

  bar(1,2,3,4,5);
}
