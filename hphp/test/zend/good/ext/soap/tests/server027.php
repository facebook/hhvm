<?hh
class Foo {

  function Foo() {
  }

  function test() {
    return $this->str;
  }
}
<<__EntryPoint>> function main(): void {
$foo = new Foo();
$server = new SoapServer(null,darray['uri'=>"http://testuri.org"]);
$server->setObject($foo);
var_dump($server->getfunctions());
echo "ok\n";
}
