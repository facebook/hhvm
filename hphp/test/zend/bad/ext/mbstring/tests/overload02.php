<?php
echo mb_internal_encoding()."\n";

$ngchars = array('ǽ','ɽ','��','��');
$str = '��Ͻ�ܻ���Һ���ɽ��ǽ��ɽ��������˽��Ž�չ�ʸ����ͽ���Ƭ���ե���';
$converted_str = mb_convert_encoding($str, 'Shift_JIS');
mb_regex_encoding('Shift_JIS');
foreach($ngchars as $c) {
	$c = mb_convert_encoding($c, 'Shift_JIS');
	$replaced = mb_convert_encoding(ereg_replace($c, '!!', $converted_str), mb_internal_encoding(), 'Shift_JIS');
	var_dump(strpos($replaced, '!!'));
}
?>
