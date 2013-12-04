<?php
var_dump(iconv_mime_encode('', ''));
var_dump(iconv_mime_encode('', '', array('line-break-chars' => 1)));
?>