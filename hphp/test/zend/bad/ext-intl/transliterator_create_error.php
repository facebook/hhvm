<?php

ini_set("intl.error_level", E_WARNING);
Transliterator::create("inexistent id");
echo intl_get_error_message(), "\n";
Transliterator::create("bad UTF-8 \x8F");
echo intl_get_error_message(), "\n";

echo "Done.\n";