<?php

ob_start();

/* 
 * Prototype : bool session_set_save_handler(SessionHandler $handler [, bool $register_shutdown_function = true])
 * Description : Sets user-level session storage functions
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_set_save_handler() : full handler implementation ***\n";

class MySession2 extends SessionHandler {
	public $path;

	public function open($path, $name) {
		if (!$path) {
			$path = sys_get_temp_dir();
		}
		$this->path = $path . '/u_sess_' . $name;
		return true;
	}

	public function close() {
		return true;
	}

	public function read($id) {
		return @file_get_contents($this->path . $id);
	}

	public function write($id, $data) {
		return file_put_contents($this->path . $id, $data);
	}

	public function destroy($id) {
		@unlink($this->path . $id);
	}

	public function gc($maxlifetime) {
		foreach (glob($this->path . '*') as $filename) {
			if (filemtime($filename) + $maxlifetime < time()) {
				@unlink($filename);
			}
		}
		return true;
	}
}

$handler = new MySession2;
session_set_save_handler(array($handler, 'open'), array($handler, 'close'),
	array($handler, 'read'), array($handler, 'write'), array($handler, 'destroy'), array($handler, 'gc'));
session_start();

$_SESSION['foo'] = "hello";

var_dump(session_id(), ini_get('session.save_handler'), $_SESSION);

session_write_close();
session_unset();

session_start();
var_dump($_SESSION);

session_write_close();
session_unset();

session_set_save_handler($handler);
session_start();

$_SESSION['foo'] = "hello";

var_dump(session_id(), ini_get('session.save_handler'), $_SESSION);

session_write_close();
session_unset();

session_start();
var_dump($_SESSION);

session_write_close();
session_unset();
