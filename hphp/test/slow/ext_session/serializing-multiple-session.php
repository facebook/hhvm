<?php

class B {}

// Multiple sessions serialized in the same session file.
$data = <<<EOF
adminhtml|a:2:{
  s:13:"session_hosts";a:1:{
    s:10:"compat.dev";b:1;
  }
  s:6:"locale";s:5:"en_US";
}
admin|a:2:{
  s:2:"U1";O:1:"B":1:{
    s:10:"\0*\0_roleId";s:2:"U1";
  }
  s:2:"U2";a:1:{
    s:8:"instance";r:6;
  }
}
EOF;
$serializedSessions = str_replace(array(" ", "\n"), "", $data);

session_start();
session_decode($serializedSessions);

// Verify that the reference still exists,
// and that it is of the right class.
var_dump(get_class($_SESSION['admin']['U2']['instance']) === 'B');

// Verify that the reserialized data is equal to the original.
var_dump(session_encode() == $serializedSessions);

session_destroy();
