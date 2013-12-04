<?php
ini_set('allow_url_include', 0);

ini_set('allow_url_fopen', 1);

class test {
    private $data = '<?php
ini_set('allow_url_include', 0);

ini_set('allow_url_fopen', 1);
 echo "Hello World\n";?>';
	private $pos;
	private $stream = null;

	function stream_open($path, $mode, $options, &$opened_path)
	{
		if (strpos($path, "test2://") === 0) {
			$this->stream = fopen("test1://".substr($path, 8), $mode);
			return !empty($this->stream);
		}
		if (strchr($mode, 'a'))
			$this->pos = strlen($this->data);
		else
			$this->po = 0;
		
		return true;
	}

	function stream_read($count)
	{
		if (!empty($this->stream)) {
			return fread($this->stream, $count);
		}
		$ret = substr($this->data, $this->pos, $count);
		$this->pos += strlen($ret);
		return $ret;
	}

	function stream_tell()
	{
		if (!empty($this->stream)) {
			return ftell($this->stream);
		}
		return $this->pos;
	}

	function stream_eof()
	{
		if (!empty($this->stream)) {
			return feof($this->stream);
		}
		return $this->pos >= strlen($this->data);
	}

	function stream_seek($offset, $whence)
	{
		if (!empty($this->stream)) {
			return fseek($this->stream, $offset, $whence);
		}
		switch($whence) {
			case SEEK_SET:
				if ($offset < $this->data && $offset >= 0) {
					$this->pos = $offset;
					return true;
				} else {
					return false;
				}
				break;
			case SEEK_CUR:
				if ($offset >= 0) {
					$this->pos += $offset;
					return true;
				} else {
					return false;
				}
				break;
			case SEEK_END:
				if (strlen($this->data) + $offset >= 0) {
					$this->pos = strlen($this->data) + $offset;
					return true;
				} else {
					return false;
				}
				break;
			default:
				return false;
		}
	}

}

stream_register_wrapper("test1", "test", STREAM_IS_URL);
stream_register_wrapper("test2", "test");
echo @file_get_contents("test1://hello"),"\n";
@include "test1://hello";
echo @file_get_contents("test2://hello"),"\n";
include "test2://hello";