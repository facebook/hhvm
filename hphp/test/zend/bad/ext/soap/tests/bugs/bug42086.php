<?php
$request = <<<EOF
<?xml version="1.0" encoding="UTF-8"?>
<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"><SOAP-ENV:Body><firstFunctionWithoutParam/></SOAP-ENV:Body></SOAP-ENV:Envelope>
EOF;

class firstFunctionWithoutParamResponse {
	public $param;
}

function firstFunctionWithoutParam() {
	$ret = new firstFunctionWithoutParamResponse();
	$ret->param	=	"firstFunctionWithoutParam";
	return $ret;
}
	
$server = new SoapServer(dirname(__FILE__).'/bug42086.wsdl',
	array('features'=>SOAP_SINGLE_ELEMENT_ARRAYS));
$server->addFunction('firstFunctionWithoutParam');
$server->handle($request);
?>