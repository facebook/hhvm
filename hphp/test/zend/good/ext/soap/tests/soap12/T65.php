<?php
$HTTP_RAW_POST_DATA = <<<EOF
<?xml version='1.0' ?>
<!DOCTYPE DOC [
<!ELEMENT Envelope (Body) >
<!ELEMENT Body (echoOk) >
<!ELEMENT echoOk (#PCDATA) >
]>
<env:Envelope xmlns:env="http://www.w3.org/2003/05/soap-envelope"> 
  <env:Body>
    <test:echoOk xmlns:test="http://example.org/ts-tests">
      foo
    </test:echoOk>
  </env:Body>
</env:Envelope>
EOF;
include "soap12-test.inc";
?>