<?php
class test {
	public $idx = 0;

	function dir_opendir($path, $options) {
		print "Opening\n";
		$this->idx = 0;

		return true;
	}

	function dir_readdir() {
		$sample = array('first','second','third','fourth');

		if ($this->idx >= count($sample)) return false;
									else  return $sample[$this->idx++];
	}

	function dir_rewinddir() {
		$this->idx = 0;

		return true;
	}

	function dir_closedir() {
		print "Closing up!\n";

		return true;
	}
}

stream_wrapper_register('test', 'test');

var_dump(scandir('test://example.com/path/to/test'));
?>