<?php

$algo = MHASH_MD5;
var_dump($algo);
var_dump(bin2hex(mhash($algo, "test")));
var_dump($algo);

?>