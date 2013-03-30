<?php
do {
	if (1) {
		echo "1: ok\n";
		goto L1;
	} else {
	    echo "bug\n";
L1:
		echo "2: ok\n";
	}
} while (0);
?>