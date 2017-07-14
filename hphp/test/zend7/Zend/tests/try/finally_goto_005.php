<?php

label: try {
	goto label;
} finally {
	print "success";
	return; // don't loop
}

?>
