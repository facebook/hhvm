<?php
$sock = socket_create_listen(80);?>
<?php
unlink(dirname(__FILE__) . '/006_root_check.tmp');