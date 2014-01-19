<?php
date_default_timezone_set("GMT");
$strCurrDate = date('Y-m-d H:i:s',strtotime('2005-06-30 21:04:23'));
$strMonAfter = date('Y-m-d H:i:s',strtotime('+1 month',strtotime($strCurrDate)));

echo "strCurrDate:$strCurrDate strMonAfter:$strMonAfter";
?>