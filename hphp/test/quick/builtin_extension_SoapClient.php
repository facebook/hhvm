<?php

include __DIR__."/builtin_extensions.inc";

class A_SoapClient extends SoapClient {
  public $___x;
}
test("SoapClient", __FILE__ . ".wsdl");
