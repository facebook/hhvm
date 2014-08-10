<?php
class testcase {
	private $encoding;
	private $bom;
	private $prologue;
	private $tags;
	private $chunk_size;

	function testcase($enc, $chunk_size = 0, $bom = 0, $omit_prologue = 0) {
		$this->encoding = $enc;
		$this->chunk_size = $chunk_size;
		$this->bom = $bom;
		$this->prologue = !$omit_prologue;
		$this->tags = array();
	}

	function start_element($parser, $name, $attrs) {
		$attrs = array_map('bin2hex', $attrs);
		$this->tags[] = bin2hex($name).": ".implode(', ', $attrs);
	}

	function end_element($parser, $name) {
	}

	function run() {
		$data = '';

		if ($this->prologue) {
			$canonical_name = preg_replace('/BE|LE/i', '', $this->encoding);
			$data .= "<?xml version=\"1.0\" encoding=\"$canonical_name\" ?>\n";
		}

		$data .= <<<HERE
<テスト:テスト1 xmlns:テスト="http://www.example.com/テスト/" テスト="テスト">
  <テスト:テスト2 テスト="テスト">
	<テスト:テスト3>
	  test! 
	</テスト:テスト3>
  </テスト:テスト2>
</テスト:テスト1>
HERE;

		$data = iconv("UTF-8", $this->encoding, $data);

		$parser = xml_parser_create(NULL);
		xml_parser_set_option($parser, XML_OPTION_CASE_FOLDING, 0);
		xml_set_element_handler($parser, "start_element", "end_element");
		xml_set_object($parser, $this);

		if ($this->chunk_size == 0) {
			$success = @xml_parse($parser, $data, true);
		} else {
			for ($offset = 0; $offset < strlen($data);
					$offset += $this->chunk_size) {
				$success = @xml_parse($parser, substr($data, $offset, $this->chunk_size), false);
				if (!$success) {
					break;
				}
			}
			if ($success) {
				$success = @xml_parse($parser, "", true);
			}
		}

		echo "Encoding: $this->encoding\n";
		echo "XML Prologue: ".($this->prologue ? 'present': 'not present'), "\n";
		echo "Chunk size: ".($this->chunk_size ? "$this->chunk_size byte(s)\n": "all data at once\n");
		echo "BOM: ".($this->bom ? 'prepended': 'not prepended'), "\n";

		if ($success) { 
			var_dump($this->tags);
		} else {
			echo "[Error] ", xml_error_string(xml_get_error_code($parser)), "\n";
		}
	}
}
$suite = array(
	new testcase("EUC-JP"  ,  0),
	new testcase("EUC-JP"  ,  1),
	new testcase("Shift_JIS", 0),
	new testcase("Shift_JIS", 1),
	new testcase("GB2312",    0),
	new testcase("GB2312",    1),
);

if (XML_SAX_IMPL == 'libxml') {
  echo "libxml2 Version => " . LIBXML_DOTTED_VERSION. "\n";
} else {
  echo "libxml2 Version => NONE\n";  
}

foreach ($suite as $testcase) {
	$testcase->run();
}

// vim600: sts=4 sw=4 ts=4 encoding=UTF-8
?>
