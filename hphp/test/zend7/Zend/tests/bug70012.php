<?php
try {
	echo "Outer try\n";
	try {
		echo "  Middle try\n";
		throw new Exception();
	} finally {
		echo "  Middle finally\n";
		try {
			echo "    Inner try\n";
		} finally {
			echo "    Inner finally\n";
		}
	}
	echo "Outer shouldnt get here\n";
} catch (Exception $e) {
	echo "Outer catch\n";
} finally {
	echo "Outer finally\n";
}
?>
