<?php
// TODO: Do real test

// EUC-JP
$euc_jp = "�ƥ��������ܸ�ʸ���󡣤��Υ⥸�塼���PHP�˥ޥ���Х��ȴؿ����󶡤��ޤ���";
mb_http_output('EUC-JP') or print("mb_http_output() failed\n");
ob_start('mb_output_handler');
echo $euc_jp;
$output = ob_get_clean();

var_dump( $output );

?>
