<?php

ob_start();

/* 
 * Prototype : bool session_set_save_handler(SessionHandler $handler [, bool $register_shutdown_function = true])
 * Description : Sets user-level session storage functions
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_set_save_handler() : incomplete implementation ***\n";

class MySession6 extends SessionHandler {
	public function open($path, $name) {
		// don't call parent
		return true;
	}

	public function read($id) {
		// should error because parent::open hasn't been called
		return parent::read($id);
	}
}

$handler = new MySession6;
session_set_save_handler($handler);
session_start();

var_dump(session_id(), ini_get('session.save_handler'), $_SESSION);

session_write_close();
session_unset();

