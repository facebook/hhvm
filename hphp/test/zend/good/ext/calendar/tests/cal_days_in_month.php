<?php
$num = cal_days_in_month(CAL_GREGORIAN, 8, 2003); 
echo "There are $num days in August 2003\n";
$num = cal_days_in_month(CAL_GREGORIAN, 2, 2003); 
echo "There are $num days in February 2003\n";
$num = cal_days_in_month(CAL_GREGORIAN, 2, 2004); 
echo "There are $num days in February 2004\n";
$num = cal_days_in_month(CAL_GREGORIAN, 12, 2034); 
echo "There are $num days in December 2034\n";
?>