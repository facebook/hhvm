<?php

function testModeTmpFile() {
  $t = tmpfile();
  $meta = stream_get_meta_data($t);
  echo "Mode of tmpfile is: {$meta['mode']}\n";
}
testModeTmpFile();

