<?php
$HTTP_RAW_POST_DATA = <<<EOF
<?xml version='1.0' ?>
<env:Envelope xmlns:env="http://www.w3.org/2003/05/soap-envelope"> 
  <env:Body>
    <test:DoesNotExist xmlns:test="http://example.org/ts-tests">
  </test:DoesNotExist>
 </env:Body>
</env:Envelope>
EOF;
include "soap12-test.inc";
?>