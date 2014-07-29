<?php
$HTTP_RAW_POST_DATA = <<<EOF
<?xml version="1.0"?>
<env:Envelope xmlns:env="http://www.w3.org/2003/05/soap-envelope"
              xmlns:xsd="http://www.w3.org/2001/XMLSchema"
              xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <env:Body>
    <test:echoFloatArray xmlns:test="http://example.org/ts-tests"
          env:encodingStyle="http://www.w3.org/2003/05/soap-encoding">
      <inputFloatArray enc:itemType="xsd:float" enc:arraySize="2"
                       xmlns:enc="http://www.w3.org/2003/05/soap-encoding">
        <item xsi:type="xsd:float">5.5</item>
        <item xsi:type="xsd:float">12999.9</item>
      </inputFloatArray>
    </test:echoFloatArray>
  </env:Body>
</env:Envelope>
EOF;
include "soap12-test.inc";
?>