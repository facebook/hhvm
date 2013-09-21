<?php
$objRfClass = new ReflectionClass('SoapClient');
$objRfMethod = $objRfClass->getMethod('__soapCall');
$arrParams = $objRfMethod->getParameters();
foreach($arrParams as $objRfParam)
{
        var_dump($objRfParam->getName());
}
?>