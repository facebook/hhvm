<?php
$sock = socket_create_listen(80);
?>
<?php error_reporting(0); ?>
<?php
unlink(dirname(__FILE__) . '/006_root_check.tmp');