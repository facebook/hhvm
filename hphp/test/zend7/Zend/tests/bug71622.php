<?php

function getMethodName(&$methodName) {
	$methodName = Abc::METHOD_NAME;
}

class Abc {
	const METHOD_NAME = "goal";

	private static function goal() {
		echo "success\n";
	}

	public static function run() {
		$method = "foobar";
		getMethodName($method);
		var_dump(is_callable("self::$method"));
		self::$method();
	}
}

Abc::run();

?>
