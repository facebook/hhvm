<?php
ini_set("intl.error_level", E_WARNING);

$fmt = <<<EOD
{foo,date}
EOD;

$mf = new MessageFormatter('en_US', $fmt);
var_dump($mf->format(array("foo" => new stdclass())));
