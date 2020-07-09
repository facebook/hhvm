<?hh

class A_SoapClient extends SoapClient {
  public $___x;
}
<<__EntryPoint>> function main(): void {
  include __DIR__."/builtin_extensions.inc";
  test("SoapClient", __FILE__ . ".wsdl");
}
