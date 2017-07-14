<?php
try {
	static::x;
} catch (Error $e) {
	var_dump($e->getMessage());
}

try {
	parent::x;
} catch (Error $e) {
	var_dump($e->getMessage());
}

try {
	self::x;
} catch (Error $e) {
	var_dump($e->getMessage());
}

try {
	new static;
} catch (Error $e) {
	var_dump($e->getMessage());
}

try {
	static::x();
} catch (Error $e) {
	var_dump($e->getMessage());
}

try {
	static::$i;
} catch (Error $e) {
	var_dump($e->getMessage());
}
?>
