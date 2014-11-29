<?php

require_once('test_base.inc');

requestAll(array(
    "test_get.php?name=Foo",
    "test_get.php?name=Bar",
    "subdoc//subdir/test.php",
    "subdoc/subdir/test.php",
));
