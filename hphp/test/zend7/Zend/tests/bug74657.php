<?php

interface I {
}

class C {
	const FOO = I::FOO;

	public $options = [self::FOO => "bar"];
}

try {
	var_dump((new C)->options);
} catch (Throwable $e) {}

var_dump((new C)->options);
?>
