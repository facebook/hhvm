<?php
/* include('test.inc'); */
/* charset=EUC-JP */

$str = "
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
���ܸ�ƥ����Ȥ�English Text
";

$str = iconv("EUC-JP", "SJIS", $str);
$str = base64_encode($str);
echo $str."\n";

?>
