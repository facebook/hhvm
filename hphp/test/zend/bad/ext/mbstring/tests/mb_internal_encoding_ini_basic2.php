<?php

echo "*** Testing INI mbstring.internal_encoding : basic functionality ***\n";

echo mb_internal_encoding()."\n";
echo ini_get('mbstring.internal_encoding')."\n";
mb_internal_encoding('UTF-8');
echo mb_internal_encoding()."\n";
echo ini_get('mbstring.internal_encoding')."\n";

?>
===DONE===