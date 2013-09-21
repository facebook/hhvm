<?php
$HTTP_RAW_POST_DATA = <<<EOF
<?xml version='1.0' ?>
<env:Envelope xmlns:env="http://wrong-version/"> 
  <env:Body>
    <test:echoOk xmlns:test="http://example.org/ts-tests">
      foo
    </test:echoOk>
  </env:Body>
</env:Envelope>
EOF;
include "soap12-test.inc";
?>