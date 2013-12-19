<?php
$stub = '<?php
Phar::interceptFileFuncs();
require "phar://this/index.php";
__HALT_COMPILER(); ?>';
$p = new Phar(__DIR__ . '/issue0115_1.phar.php', 0, 'this');
$p['index.php'] = '<?php
echo "Hello from Index 1.\n";
require_once "phar://this/hello.php"; 
';
$p['hello.php'] = "Hello World 1!\n";    
$p->setStub($stub);
unset($p);
$p = new Phar(__DIR__ . '/issue0115_2.phar.php', 0, 'this');
$p['index.php'] = '<?php
echo "Hello from Index 2.\n";
require_once "phar://this/hello.php"; 
';
$p['hello.php'] = "Hello World 2!\n";    
$p->setStub($stub);
unset($p);

include "php_cli_server.inc";
php_cli_server_start('-d opcache.enable=1 -d opcache.enable_cli=1');
echo file_get_contents('http://' . PHP_CLI_SERVER_ADDRESS . '/issue0115_1.phar.php');
echo file_get_contents('http://' . PHP_CLI_SERVER_ADDRESS . '/issue0115_2.phar.php');
?>
<?php 
@unlink(__DIR__ . '/issue0115_1.phar.php');
@unlink(__DIR__ . '/issue0115_2.phar.php');
?>