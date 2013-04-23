<?php /* $Id$ */
$text = 'HctBDoAgDETRq8zOjfEeHKOGATG0TRpC4u1Vdn/xX4IoxkVMxgP1zA4vkJVhULk9UGkM6TvSNolmxUNlNLePVQ45O3eINf0fsQxtCxwv';

$fp = fopen('php://stdout', 'w');
stream_filter_append($fp, 'convert.base64-decode', STREAM_FILTER_WRITE);
stream_filter_append($fp, 'zlib.inflate', STREAM_FILTER_WRITE);
fwrite($fp, $text);
fclose($fp);

?> 