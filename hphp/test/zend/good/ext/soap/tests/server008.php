<?hh
class Foo {

  function Foo() :mixed{
  }

  function test() :mixed{
    return $this->str;
  }
}
<<__EntryPoint>> function main(): void {
$server = new SoapServer(null,dict['uri'=>"http://testuri.org"]);
$server->setClass("Foo");
var_dump($server->getfunctions());
echo "ok\n";
}
