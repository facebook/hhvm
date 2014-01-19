<?php
/* include('test.inc'); */
iconv_set_encoding('internal_encoding', 'EUC-JP');
iconv_set_encoding('output_encoding', 'Shift_JIS');
ob_start('ob_iconv_handler');
print "あいうえお";
ob_end_flush();
?>