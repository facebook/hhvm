<?php
try {
Phar::webPhar('oopsiedaisy.phar', '/index.php');
} catch (Exception $e) {
echo $e->getMessage() . "\n";
}
__HALT_COMPILER();
?>