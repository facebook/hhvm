<?php
$HTTP_RAW_POST_DATA = <<<EOF
<?xml version='1.0' ?>
<env:Envelope xmlns:env="http://www.w3.org/2003/05/soap-envelope"> 
 <env:Header>
  <test:echoResolvedRef xmlns:test="http://example.org/ts-tests"
        env:role="http://www.w3.org/2003/05/soap-envelope/role/next"
        env:mustUnderstand="1">
    <test:RelativeReference xml:base="http://example.org/today/"
          xlink:href="new.xml"
          xmlns:xlink="http://www.w3.org/1999/xlink" />
  </test:echoResolvedRef>
 </env:Header>
 <env:Body>
 </env:Body>
</env:Envelope>
EOF;
include "soap12-test.inc";
?>