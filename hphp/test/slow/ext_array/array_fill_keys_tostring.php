<?php

class StringableObj { function __toString() { return 'Hello'; } }
class UnstringableObj { }

function main() {
  $keys = array(
    'stringable_obj' => new StringableObj(),
    'array' => array(),
    'bool true' => true,
    'bool false' => false,
    'double' => 3.141,
    'null' => null,
    // Last, as it should fatal
    'unstringable_obj' => new UnstringableObj()
  );

  foreach ($keys as $desc => $key) {
    printf("---%s---\n", $desc);
    var_dump(array_fill_keys(array($key), 'value'));
  }
}

main();
