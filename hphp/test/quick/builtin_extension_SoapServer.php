<?hh

class A_SoapServer extends SoapServer {
  public $___x;
}
<<__EntryPoint>> function main(): void {
  include __DIR__."/builtin_extensions.inc";
  test("SoapServer", __DIR__ . "/builtin_extension_SoapClient.php.wsdl");
}
