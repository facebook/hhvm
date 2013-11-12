<?php

$array = array('k1' => array());

$it = new RecursiveIteratorIterator(
  new RecursiveArrayIterator($array)
);

for (true; $it->valid(); $it->next()) {
  var_export(
    array(
      'key' => $it->key(),
      'value' => $it->current(),
    )
  );
}
print("\n===DONE===\n");
