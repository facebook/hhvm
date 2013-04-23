<?php
$td = mcrypt_module_open('rijndael-256', '', 'ofb', '');
mcrypt_generic($td, "foobar");
mdecrypt_generic($td, "baz");
?>