<?php
$client = new SoapClient(dirname(__FILE__) . '/bug47049.wsdl',
	array('trace' => 1 , 'exceptions' => 0));
$host = array('uuid' => 'foo');
$software_list = array(array('name' => 'package', 'version' => '1.2.3', 'state' => 'installed'));
$updates = array();
$report_id = $client->__soapCall('reportSoftwareStatus',array($host, $software_list, $updates));
echo $client->__getLastRequest();
?>