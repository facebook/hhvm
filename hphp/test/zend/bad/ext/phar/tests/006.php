<?php
try {
Phar::mapPhar('hio');
} catch (Exception $e) {
echo $e->getMessage();
}
__HALT_COMPILER(); ?>