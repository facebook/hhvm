<?hh

include __DIR__."/builtin_extensions.inc";

class A_SoapClient extends SoapClient {
  public $___x;
}
<<__EntryPoint>> function main(): void {
test("SoapClient", __FILE__ . ".wsdl");
}
