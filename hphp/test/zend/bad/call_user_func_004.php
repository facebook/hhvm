<?php

class foo {
	public function teste() {
		$this->a = 1;
	}
}

call_user_func(array('foo', 'teste'));

?>