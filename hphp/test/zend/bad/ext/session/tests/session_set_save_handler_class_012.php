<?php

ob_start();

/* 
 * Prototype : bool session_set_save_handler(SessionHandler $handler [, bool $register_shutdown_function = true])
 * Description : Sets user-level session storage functions
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_set_save_handler() : incorrect arguments for existing handler open ***\n";

class MySession extends SessionHandler {
	public $i = 0;
	public function open($path, $name) {
		++$this->i;
		echo 'Open ', session_id(), "\n";
		return parent::open();
	}
	public function read($key) {
		++$this->i;
		echo 'Read ', session_id(), "\n";
		return parent::read($key);
	}
}

$oldHandler = ini_get('session.save_handler');
$handler = new MySession;
session_set_save_handler($handler);
session_start();

var_dump(session_id(), $oldHandler, ini_get('session.save_handler'), $handler->i, $_SESSION);

