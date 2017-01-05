<?php
$rc = new ReflectionClass('XsltProcessor');
echo $rc->getMethod('setSecurityPrefs')->invoke(new XmlReader(), 0xdeadbeef);
