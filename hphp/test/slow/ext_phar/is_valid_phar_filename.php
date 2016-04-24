<?php
// Test case from PHP source (php-src/ext/phar/tests/phar_isvalidpharfilename.phpt)
function err_handler($errno, $errstr, $errfile, $errline) {
  echo "Catchable fatal error: $errstr in $errfile on line $errline\n";
  throw new Exception;
}

set_error_handler('err_handler', E_RECOVERABLE_ERROR);

chdir(__DIR__);

try {
  Phar::isValidPharFilename([]);
} catch (Exception $e) {}

echo "*\n";
var_dump(Phar::isValidPharFilename('*'));
var_dump(Phar::isValidPharFilename('*', true));
var_dump(Phar::isValidPharFilename('*', false));

echo "\nboo.phar\n";
var_dump(Phar::isValidPharFilename('boo.phar'));
var_dump(Phar::isValidPharFilename('boo.phar', true));
var_dump(Phar::isValidPharFilename('boo.phar', false));

echo "\nboo.tar\n";
var_dump(Phar::isValidPharFilename('boo.tar'));
var_dump(Phar::isValidPharFilename('boo.tar', true));
var_dump(Phar::isValidPharFilename('boo.tar', false));

echo "\nboo.phar.tar\n";
var_dump(Phar::isValidPharFilename('boo.phar.tar'));
var_dump(Phar::isValidPharFilename('boo.phar.tar', true));
var_dump(Phar::isValidPharFilename('boo.phar.tar', false));

mkdir(__DIR__.'/.phar');

echo "\n.phar/boo.tar\n";
var_dump(Phar::isValidPharFilename('.phar/boo.tar'));
var_dump(Phar::isValidPharFilename('.phar/boo.tar', true));
var_dump(Phar::isValidPharFilename('.phar/boo.tar', false));

echo "\n.phar.tar\n";
var_dump(Phar::isValidPharFilename('.phar.tar'));
var_dump(Phar::isValidPharFilename('.phar.tar', true));
var_dump(Phar::isValidPharFilename('.phar.tar', false));

echo "\n.phar.phar\n";
var_dump(Phar::isValidPharFilename('.phar.phar'));
var_dump(Phar::isValidPharFilename('.phar.phar', true));
var_dump(Phar::isValidPharFilename('.phar.phar', false));

echo "\n.phar.phart\n";
var_dump(Phar::isValidPharFilename('.phar.phart'));
var_dump(Phar::isValidPharFilename('.phar.phart', true));
var_dump(Phar::isValidPharFilename('.phar.phart', false));

echo "\nmy.pharmy\n";
var_dump(Phar::isValidPharFilename('my.pharmy'));
var_dump(Phar::isValidPharFilename('my.pharmy', true));
var_dump(Phar::isValidPharFilename('my.pharmy', false));

echo "\nphar.zip\n";
var_dump(Phar::isValidPharFilename('phar.zip'));
var_dump(Phar::isValidPharFilename('phar.zip', true));
var_dump(Phar::isValidPharFilename('phar.zip', false));

echo "\nphar.zip.phar\n";
var_dump(Phar::isValidPharFilename('phar.zip.phar'));
var_dump(Phar::isValidPharFilename('phar.zip.phar', true));
var_dump(Phar::isValidPharFilename('phar.zip.phar', false));

echo "\ndir.phar.php\n";
var_dump(Phar::isValidPharFilename('dir.phar.php'));
var_dump(Phar::isValidPharFilename('dir.phar.php', true));
var_dump(Phar::isValidPharFilename('dir.phar.php', false));

rmdir(__DIR__.'/.phar');
