<?php
function my_error_handler($errno, $errmsg, $filename, $linenum, $vars)
{
	echo "$errno: $errmsg\n";
}
set_error_handler('my_error_handler');
$preference = array(
	"scheme" => "B",
	"output-charset" => "ISO-2022-JP",
	"input-charset" => "EUC-JP",
	"line-break-chars" => "\n"
);
for ($line_len= 0; $line_len < 80; ++$line_len) {
	print "-------- line length=$line_len\n";
	$preference["line-length"] = $line_len;
	$result = iconv_mime_encode("From", "サンプル文字列サンプル文字列日本語テキスト", $preference);
	var_dump($result);
	if ($result !== false) {
                $max = max(array_map("strlen", explode("\n", $result)));
		print "-------- ";
		var_dump(($max <= $line_len));
	} else {
		print "-------- \n";
	}
}
?>