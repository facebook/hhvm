<?php

function test() {
  var_dump($x ?? 3);
  var_dump(null ?? 3);
  var_dump(0 ?? 3);
  var_dump(false ?? 3);
  var_dump('' ?? 3);
}

test();
