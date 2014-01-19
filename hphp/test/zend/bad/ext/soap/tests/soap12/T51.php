<?php
$HTTP_RAW_POST_DATA = <<<EOF
<?xml version="1.0"?>
<env:Envelope xmlns:env="http://www.w3.org/2003/05/soap-envelope"
              xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
              xmlns:xsd="http://www.w3.org/2001/XMLSchema">
  <env:Body>
    <test:echoBase64 xmlns:test="http://example.org/ts-tests"
       env:encodingStyle="http://www.w3.org/2003/05/soap-encoding">
      <inputBase64 xsi:type="xsd:base64Binary">
        YUdWc2JHOGdkMjl5YkdRPQ==
      </inputBase64>
    </test:echoBase64>
  </env:Body>
</env:Envelope>
EOF;
include "soap12-test.inc";
?>