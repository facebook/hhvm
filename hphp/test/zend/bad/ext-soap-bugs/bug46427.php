<?php
error_reporting(E_ALL|E_STRICT);

function getSoapClient_1() {
    $ctx = stream_context_create();
    return new SoapClient(NULL, array(
    	'stream_context' => $ctx,
    	'location' => 'test://',
    	'uri' => 'test://',
    	'exceptions' => false));
}

getSoapClient_1()->__soapCall('Help', array());
echo "ok\n";
?>