<?php

class Loader {
	private $position;
	private $data;
	public function stream_open($path, $mode, $options, &$opened_path)  {
		$this->data = '<' . "?php \n\"\";ll l\n ?" . '>';
		$this->position = 0;
		return true;
	}
	function stream_read($count) {
		$ret = substr($this->data, $this->position, $count);
		$this->position += strlen($ret);
		return $ret;
	}
	function stream_eof() {
		return $this->position >= strlen($this->data);
	}
	function stream_stat() {
		return array('size' => strlen($this->data));
	}
}
stream_wrapper_register('Loader', 'Loader');
require 'Loader://qqq.php';

echo "Done\n";
?>