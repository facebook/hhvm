<?php

echo date('Y-m-d H:i:s T', strtotime('2004-10-31 EDT +1 hour'))."\n";
echo date('Y-m-d H:i:s T', strtotime('2004-10-31 EDT +2 hours'))."\n";
echo date('Y-m-d H:i:s T', strtotime('2004-10-31 EDT +3 hours'))."\n";

echo "\n";

echo date('Y-m-d H:i:s T', strtotime('2004-10-31 +1 hour'))."\n";
echo date('Y-m-d H:i:s T', strtotime('2004-10-31 +2 hours'))."\n";
echo date('Y-m-d H:i:s T', strtotime('2004-10-31 +3 hours'))."\n";
?>