<?php
include dirname(__FILE__)."/../../../../sapi/cli/tests/php_cli_server.inc";
php_cli_server_start(file_get_contents(dirname(__FILE__).'/bug64433_srv.inc'));

echo file_get_contents("http://".PHP_CLI_SERVER_ADDRESS."/index.php");
echo "default\n";
$codes = array(200, 201, 204, 301, 302, 303, 304, 305, 307, 404, 500);
foreach($codes as $code) {
	echo "$code: ".file_get_contents("http://".PHP_CLI_SERVER_ADDRESS."/index.php?status=$code&loc=1");
}
echo "follow=0\n";
$arr = array('http'=>
                        array(
                                'follow_location'=>0,	
                        )
                );
$context = stream_context_create($arr);
foreach($codes as $code) {
	echo "$code: ".file_get_contents("http://".PHP_CLI_SERVER_ADDRESS."/index.php?status=$code&loc=1", false, $context);
}
echo "follow=1\n";
$arr = array('http'=>
                        array(
                                'follow_location'=>1,	
                        )
                );
$context = stream_context_create($arr);
foreach($codes as $code) {
	echo "$code: ".file_get_contents("http://".PHP_CLI_SERVER_ADDRESS."/index.php?status=$code&loc=1", false, $context);
}