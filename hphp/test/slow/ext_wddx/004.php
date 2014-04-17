<?php
	class foo {
		public $bar = "ok";
		public $invisible = 'you don\'t see me!';

		function method() { $this->yes = "done"; }

		public function __sleep() { return array('bar', 'yes'); }
	}

	session_start();

	$_SESSION['data'] = array(
		'test1' => true,
		'test2' => 'some string',
		'test3' => 654321,
		'test4' => array(
			'some string',
			true,
			null
		),
	);

	$_SESSION['class'] = new foo();
	$_SESSION['class']->method();

	var_dump(session_encode());

	session_destroy();
?>
