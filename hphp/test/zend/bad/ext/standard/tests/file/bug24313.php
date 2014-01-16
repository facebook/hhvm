<?php
ini_set('open_basedir', /dev);

	var_dump(file_exists("/dev/bogus_file_no_such_thing"));
?>