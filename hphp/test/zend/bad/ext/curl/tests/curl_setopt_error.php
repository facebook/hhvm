<?php
echo "*** curl_setopt() call with incorrect parameters\n";
$ch = curl_init();
curl_setopt();
curl_setopt(false);

curl_setopt($ch);
curl_setopt($ch, false);
curl_setopt($ch, -10);
curl_setopt($ch, '');
curl_setopt($ch, 1, false);

curl_setopt(false, false, false);
curl_setopt($ch, '', false);
curl_setopt($ch, 1, '');
curl_setopt($ch, -10, 0);
?>