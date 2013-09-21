<?php
// by legolas558

$regex = '/(insert|drop|create|select|delete|update)([^;\']*('."('[^']*')+".')?)*(;|$)/i';

$sql = 'SELECT * FROM #__components';

if (preg_match($regex,$sql, $m)) echo 'matched';
else echo 'not matched';

print_r($m);

?>