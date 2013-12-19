<?php
var_dump(ini_set('phar.require_hash', 1));
var_dump(ini_set('phar.readonly', 1));
var_dump(ini_get('phar.require_hash'));
var_dump(ini_get('phar.readonly'));
if (version_compare(PHP_VERSION, "5.3", "<")) {
var_dump(false, false);
} else {
var_dump(ini_set('phar.require_hash', 0));
var_dump(ini_set('phar.readonly', 0));
}
var_dump(ini_get('phar.require_hash'));
var_dump(ini_get('phar.readonly'));
__HALT_COMPILER();
?>