<?php
ini_set('variables_order', GPS);

ini_set('register_argc_argv', On);

	var_dump(getopt("2a:vcd1", array("another:", 12, 0, 1, "v")));
?>