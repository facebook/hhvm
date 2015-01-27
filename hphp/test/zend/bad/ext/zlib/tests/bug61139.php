<?php

gzopen('someFile', 'c');
?>
<?php error_reporting(0); ?>
<?php 
	unlink('someFile'); 
?>