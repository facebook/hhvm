<?php
var_dump(ini_set('phar.require_hash', 1));
var_dump(ini_set('phar.readonly', 1));
var_dump(ini_get('phar.require_hash'));
var_dump(ini_get('phar.readonly'));
ini_set('phar.require_hash', 0);
ini_set('phar.readonly', 0);
var_dump(Phar::canWrite());
?>
yes
<?php
var_dump(ini_set('phar.require_hash', 'yes'));
var_dump(ini_set('phar.readonly', 'yes'));
var_dump(ini_get('phar.require_hash'));
var_dump(ini_get('phar.readonly'));
var_dump(Phar::canWrite());
ini_set('phar.require_hash', 0);
ini_set('phar.readonly', 0);
?>
on
<?php
var_dump(ini_set('phar.require_hash', 'on'));
var_dump(ini_set('phar.readonly', 'on'));
var_dump(ini_get('phar.require_hash'));
var_dump(ini_get('phar.readonly'));
var_dump(Phar::canWrite());
ini_set('phar.require_hash', 0);
ini_set('phar.readonly', 0);
?>
true
<?php
var_dump(ini_set('phar.require_hash', 'true'));
var_dump(ini_set('phar.readonly', 'true'));
var_dump(Phar::canWrite());
var_dump(ini_get('phar.require_hash'));
var_dump(ini_get('phar.readonly'));
?>
0
<?php
var_dump(ini_set('phar.require_hash', 0));
var_dump(ini_set('phar.readonly', 0));
var_dump(Phar::canWrite());
var_dump(ini_get('phar.require_hash'));
var_dump(ini_get('phar.readonly'));
?>
===DONE===