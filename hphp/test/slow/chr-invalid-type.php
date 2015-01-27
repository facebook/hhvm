<?php

$inputs = [
  -1, 0, 1, 2, 128, 255,
  "0string", "1string", "2",
  [], [1], [1,2],
  new stdClass,
  new SimpleXMLElement("<foo />"),
  new SimpleXMLElement("<foo><bar/></foo>"),
  STDIN, STDOUT, STDERR,
];

foreach ($inputs as $v) {
  var_dump(bin2hex(chr($v)));
}
