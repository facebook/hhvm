<?php

$types = array(
  'php://stdin',
  'php://stdout',
  'php://stderr',
  'php://fd/0',
  'php://temp',
  'php://memory',
  'php://input',
  'php://output',
);

foreach ($types as $type) {
  var_dump(get_resource_type(fopen($type, 'r')));
}

$tmp = tempnam(sys_get_temp_dir(), 'a');
var_dump(get_resource_type(bzopen($tmp, 'w')));
var_dump(get_resource_type(gzopen($tmp, 'w')));
var_dump(get_resource_type(imagecreate(110, 20)));
var_dump(get_resource_type(curl_init()));
