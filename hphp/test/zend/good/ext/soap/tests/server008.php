<?hh
class Foo {

  function Foo() {
  }

  function test() {
    return $this->str;
  }
}
<<__EntryPoint>> function main(): void {
$server = new SoapServer(null,darray['uri'=>"http://testuri.org"]);
$server->setClass("Foo");
var_dump($server->getfunctions());
echo "ok\n";
}
