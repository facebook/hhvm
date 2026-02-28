<?hh
class Foo {

  function Foo() :mixed{
  }

  function test() :mixed{
    return $this->str;
  }
}
<<__EntryPoint>> function main(): void {
$foo = new Foo();
$server = new SoapServer(null,dict['uri'=>"http://testuri.org"]);
$server->setobject($foo);
var_dump($server->getfunctions());
echo "ok\n";
}
