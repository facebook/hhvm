<?php

$streams = array(
  "data://invalid,data",
  "data://invalid-data",
  "data://foo;base64,data",
  "data://;base640,data",
  "data://;foobar,data",
  "data://;text/plain;base64,data"
);

foreach ($streams as $stream) {
  var_dump(file_get_contents($stream));
}
