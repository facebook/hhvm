<?hh
class Foo {

  function Foo() {
  }

  function test() {
    return $this->str;
  }
}

$server = new soapserver(null,darray['uri'=>"http://testuri.org"]);
$server->setclass("Foo");
var_dump($server->getfunctions());
echo "ok\n";
