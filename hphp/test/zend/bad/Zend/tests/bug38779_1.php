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
	function stream_flush() {
		@unlink(dirname(__FILE__)."/bug38779.txt");
		var_dump("flush!");
	}
	function stream_close() {
		var_dump("close!");
	}
}
stream_wrapper_register('Loader', 'Loader');
$fp = fopen ('Loader://qqq.php', 'r');

$filename = dirname(__FILE__)."/bug38779.txt";
$fp1 = fopen($filename, "w");
fwrite($fp1, "<"."?php blah blah?".">");
fclose($fp1);

include $filename;

echo "Done\n";
?>
