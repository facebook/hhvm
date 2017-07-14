<?php

function foo() : array {
	try {
		throw new Exception("xxxx");
	} finally {
		return null;
	}
}

foo();
?>
