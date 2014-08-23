<?php
error_reporting(E_ALL & ~E_USER_NOTICE & ~E_NOTICE);

echo "== No Search ==\n";

$it = new APCIterator('user');
apc_store('sample', 'x');
apc_store('another_sample', 'x');

foreach ($it as $key=>$val) {
  // skip to end
}

var_dump(
  $it->valid(),
  $it->key(),
  $it->current(),
  $it->next()
);


echo "== Search ==\n";

$it = new APCIterator('user', '/^b/');
// No elements, end?

var_dump(
  $it->valid(),
  $it->key(),
  $it->current(),
  $it->next()
);
