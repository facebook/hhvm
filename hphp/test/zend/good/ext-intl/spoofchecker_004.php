<?php

$korean = "\xED\x95\x9C" . "\xEA\xB5\xAD" . "\xEB\xA7\x90";

$x = new Spoofchecker();
echo "Check with default settings\n";
var_dump($x->areConfusable("HELLO", "H\xD0\x95LLO"));
var_dump($x->areConfusable("hello", "h\xD0\xB5llo"));

echo "Change confusable settings\n";
$x->setChecks(Spoofchecker::MIXED_SCRIPT_CONFUSABLE |
  Spoofchecker::WHOLE_SCRIPT_CONFUSABLE |
  Spoofchecker::SINGLE_SCRIPT_CONFUSABLE);
var_dump($x->areConfusable("HELLO", "H\xD0\x95LLO"));
var_dump($x->areConfusable("hello", "h\xD0\xB5llo"));
?>