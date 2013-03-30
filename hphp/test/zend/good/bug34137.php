<?php
$arr1 = array('a1' => array('alfa' => 'ok'));
$arr1 =& $arr1['a1'];
echo '-'.$arr1['alfa']."-\n";
?>