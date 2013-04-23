<?php
    class XSoapClient extends SoapClient {
        function __doRequest($request, $location, $action, $version, $one_way=false) {
            echo self::$crash;
        }   
    }   
    $client = new XSoapClient(null, array('uri'=>'', 'location'=>''));
    $client->__soapCall('', array());
?>