<?hh
class Foo {

  function Foo() {
  }

  function test() {
    return $this->str;
  }
}
<<__EntryPoint>> function main(): void {
$server = new soapserver(null,darray['uri'=>"http://testuri.org"]);
$server->setclass("Foo");
var_dump($server->getfunctions());
echo "ok\n";
}
