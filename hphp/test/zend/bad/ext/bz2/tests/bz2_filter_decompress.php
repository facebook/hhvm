<?php /* $Id$ */
$text = 'QlpoNDFBWSZTWRN6QG0AAAoVgECFACA395UgIABIintI1N6mpowIQ0E1MTTAQGYTNcRyMZm5kgW3ib7hVboE7Tmqj3ToGZ5G3q1ZauD2G58hibSck8KS95EEAbx1Cn+LuSKcKEgJvSA2gA==';

$fp = fopen('php://stdout', 'w');
stream_filter_append($fp, 'convert.base64-decode', STREAM_FILTER_WRITE);
stream_filter_append($fp, 'bzip2.decompress', STREAM_FILTER_WRITE);
fwrite($fp, $text);
fclose($fp);

?> 