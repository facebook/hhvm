<?php
$m = ICONV_MIME_DECODE_CONTINUE_ON_ERROR;

var_dump(iconv_mime_decode("Legal encoded-word: =?utf-8?B?Kg==?= .", $m));
var_dump(iconv_mime_decode("Legal encoded-word: =?utf-8?Q?*?= .", $m));
var_dump(iconv_mime_decode("Illegal encoded-word: =?utf-8?B?".chr(0xA1)."?= .", $m));
var_dump(iconv_mime_decode("Illegal encoded-word: =?utf-8?Q?".chr(0xA1)."?= .", $m));

var_dump(iconv_mime_decode("Legal encoded-word: =?utf-8?B?Kg==?= ."));
var_dump(iconv_mime_decode("Legal encoded-word: =?utf-8?Q?*?= ."));
var_dump(iconv_mime_decode("Illegal encoded-word: =?utf-8?B?".chr(0xA1)."?= ."));
var_dump(iconv_mime_decode("Illegal encoded-word: =?utf-8?Q?".chr(0xA1)."?= ."));
?>