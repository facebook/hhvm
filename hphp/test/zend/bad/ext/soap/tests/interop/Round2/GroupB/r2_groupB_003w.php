<?php
$param = array(
    array('row0col0', 'row0col1', 'row0col2'),
    array('row1col0', 'row1col1', 'row1col2'));
$client = new SoapClient(dirname(__FILE__)."/round2_groupB.wsdl",array("trace"=>1,"exceptions"=>0));
$client->echo2DStringArray($param);
echo $client->__getlastrequest();
$HTTP_RAW_POST_DATA = $client->__getlastrequest();
include("round2_groupB.inc");
echo "ok\n";
?>