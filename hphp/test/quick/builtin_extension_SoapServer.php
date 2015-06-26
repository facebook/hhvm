<?php

include __DIR__."/builtin_extensions.inc";

class A_SoapServer extends SoapServer {
  public $___x;
}
test("SoapServer", __DIR__ . "/builtin_extension_SoapClient.php.wsdl");
