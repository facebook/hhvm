<?php
function test() {
  $aUser = new User();
  $aUser->sName = 'newUser';

  $aUsers = Array();
  $aUsers[] = $aUser;
  $aUsers[] = $aUser;
  $aUsers[] = $aUser;
  $aUsers[] = $aUser;
  return $aUsers;
}

/* Simple User definition */
Class User {
  /** @var string */
  public $sName;
}

$server = new soapserver(null,array('uri'=>"http://testuri.org", 'soap_version'=>SOAP_1_2));
$server->addfunction("test");

$HTTP_RAW_POST_DATA = <<<EOF
<?xml version="1.0" encoding="ISO-8859-1"?>
<SOAP-ENV:Envelope
  SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"
  xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/"
  xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xmlns:si="http://soapinterop.org/xsd">
  <SOAP-ENV:Body>
    <ns1:test xmlns:ns1="http://testuri.org" />
  </SOAP-ENV:Body>
</SOAP-ENV:Envelope>
EOF;
ob_start();
$server->handle($HTTP_RAW_POST_DATA);
echo "ok\n";

$HTTP_RAW_POST_DATA = <<<EOF
<?xml version="1.0" encoding="UTF-8"?>
<env:Envelope xmlns:env="http://www.w3.org/2003/05/soap-envelope" 
  xmlns:ns1="http://testuri.org" 
  xmlns:xsd="http://www.w3.org/2001/XMLSchema"
  xmlns:enc="http://www.w3.org/2003/05/soap-encoding">
  <env:Body>
    <ns1:test env:encodingStyle="http://www.w3.org/2003/05/soap-encoding"/>
  </env:Body>
</env:Envelope>
EOF;
$server->handle($HTTP_RAW_POST_DATA);
echo "ok\n";
ob_flush();