<?php

$korean = "\xED\x95\x9C" . "\xEA\xB5\xAD" . "\xEB\xA7\x90";

$x = new Spoofchecker();
echo "Is suspcious, en_US\n";

$x->setAllowedLocales('en_US');
var_dump($x->isSuspicious($korean));

echo "Is suspcious, ko_KR\n";

$x->setAllowedLocales('en_US, ko_KR');
var_dump($x->isSuspicious($korean));
?>
