<?php
ini_set("intl.error_level", E_WARNING);

$fmt = <<<EOD
{foo,number} {foo}
EOD;

$mf = new MessageFormatter('en_US', $fmt);
var_dump($mf->format(array(7)));
