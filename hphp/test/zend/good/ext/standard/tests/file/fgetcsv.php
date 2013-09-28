<?php
	$list = array(
		'aaa,bbb',
		'aaa,"bbb"',
		'"aaa","bbb"',
		'aaa,bbb',
		'"aaa",bbb',
		'"aaa",   "bbb"',
		',',
		'aaa,',
		',"aaa"',
		'"",""',
		'"\\"","aaa"',
		'"""""",',
		'""""",aaa',
		'"\\""",aaa',
		'aaa,"\\"bbb,ccc',
		'aaa,bbb   ',
		'aaa,"bbb   "',
		'aaa"aaa","bbb"bbb',
		'aaa"aaa""",bbb',
		'aaa"\\"a","bbb"'
	);

	$file = dirname(__FILE__) . 'fgetcsv.csv';
	@unlink($file);
	foreach ($list as $v) {
		$fp = fopen($file, "w");
		fwrite($fp, $v . "\n");
		fclose($fp);

		var_dump(fgetcsv(fopen($file, "r"), 1024));
	}
	@unlink($file);
?>