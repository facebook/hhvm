<?php
try {
	$client = new SoapClient('http://i_dont_exist.com/some.wsdl');
	echo "?\n";
} catch (SoapFault $e) {
	echo get_class($e)."\n";
	echo $e->faultstring."\n";
	echo "ok\n";
} catch (Exception $e) {
	echo get_class($e)."\n";
}
?>