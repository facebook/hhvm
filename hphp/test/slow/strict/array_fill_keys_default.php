<?php


<<__EntryPoint>>
function main_array_fill_keys_default() {
$keys = array(
  1234,
  'foo',
  1.234,
);

var_dump(array_fill_keys($keys, 'foo'));
}
