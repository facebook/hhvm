<?php
ini_set('open_basedir', .);


echo tempnam("directory_that_not_exists", "prefix_");

?>