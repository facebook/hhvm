<?php
ini_set('allow_url_fopen', 1);


$streams = array(
	"data:,012345",
	);

foreach($streams as $stream)
{
	echo "===$stream===\n";

	$fp = fopen($stream, 'rb');
	
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===S:4,S===\n";
	var_dump(fseek($fp, 4));
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===GETC===\n";
	var_dump(fgetc($fp));
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===GETC===\n";
	var_dump(fgetc($fp));
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===REWIND===\n";
	var_dump(rewind($fp));
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===GETC===\n";
	var_dump(fgetc($fp));
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===S:3,S===\n";
	var_dump(fseek($fp, 3, SEEK_SET));
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===S:1,C===\n";
	var_dump(fseek($fp, 1, SEEK_CUR));
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===S:-2,C===\n";
	var_dump(fseek($fp, -2, SEEK_CUR));
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===S:-10,C===\n";
	var_dump(fseek($fp, -10, SEEK_CUR));
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===S:3,S===\n";
	var_dump(fseek($fp, 3, SEEK_SET));
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===S:10,C===\n";
	var_dump(fseek($fp, 10, SEEK_CUR));
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===S:-1,E===\n";
	var_dump(fseek($fp, -1, SEEK_END));
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===S:0,E===\n";
	var_dump(fseek($fp, 0, SEEK_END));
	var_dump(ftell($fp));
	var_dump(feof($fp));
	echo "===S:1,E===\n";
	var_dump(fseek($fp, 1, SEEK_END));
	var_dump(ftell($fp));
	var_dump(feof($fp));

	fclose($fp);
}

?>
===DONE===
<?php
ini_set('allow_url_fopen', 1);
 exit(0); ?>