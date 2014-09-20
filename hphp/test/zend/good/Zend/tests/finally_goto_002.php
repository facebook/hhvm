<?php
function foo() {
	try {
		goto test;
    } finally {
test:
    }
}
?>
