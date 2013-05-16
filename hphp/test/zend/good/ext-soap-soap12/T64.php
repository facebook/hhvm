<?php
$HTTP_RAW_POST_DATA = <<<EOF
<?xml version='1.0' ?>
<!DOCTYPE DOC [
<!NOTATION application_xml SYSTEM 'http://www.isi.edu/in-notes/iana/assignments/media-types/application/xml'>
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