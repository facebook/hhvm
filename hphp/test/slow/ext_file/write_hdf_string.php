<?php

$hdf = array(
  'bool' => false,
  'string' => 'asd',
  'num' => 12345,
  'arr' => array('bool' => false, 'string' => 'blah', 'num2' => 6789)
);
var_dump(write_hdf_string($hdf));
