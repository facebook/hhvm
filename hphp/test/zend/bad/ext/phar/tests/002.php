<?php
try {
Phar::mapPhar(5, 'hio', 'hi');

Phar::mapPhar();
} catch (Exception $e) {
	echo $e->getMessage();
}
__HALT_COMPILER(); ?>