<?php

require_once('test_base.inc');

requestAll(array(
  "test_get.php?name=Foo",
  "test_get.php?name=Bar",
  "apc_apache_note.php",
  "apc_apache_note.php",
  "apc_apache_note.php",
));
