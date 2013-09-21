<?php
/* check this works and actually returns the boolean value */
var_dump(Phar::canCompress() == (
	extension_loaded("zlib") || extension_loaded("bz2")
	));
?>