<?php
ini_set("intl.error_level", E_WARNING);

$fmt = <<<EOD
{foo}
EOD;

$mf = new MessageFormatter('en_US', $fmt);
var_dump($mf->format(array("foo" => 7, "\x80" => "bar")));

var_dump($mf->format(array("foo" => "\x80")));
