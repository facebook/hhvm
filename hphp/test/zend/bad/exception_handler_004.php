<?php

set_exception_handler("fo");
set_exception_handler(array("", ""));
set_exception_handler();
set_exception_handler("foo", "bar");

echo "Done\n";
?>