<?php
ini_set('open_basedir', .);

require_once "open_basedir.inc";
create_directories();

var_dump(file_exists('./test/ok/ok.txt'));
var_dump(file_exists('./test/foo'));

$file = str_repeat('x', 2 * PHP_MAXPATHLEN);
var_dump(file_exists("./test/$file"));
?>
<?php
require_once "open_basedir.inc";
delete_directories();
?>