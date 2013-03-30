<?php
try {
	throw new Exception();
} catch (Exception $e) {
	gc_collect_cycles();
}
echo "ok\n";