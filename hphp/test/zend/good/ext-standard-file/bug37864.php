<?php
	$tmpfname = tempnam(sys_get_temp_dir(), "emptyfile");
	var_dump(file_get_contents($tmpfname));
	echo "done.\n";
	unlink($tmpfname);
?>