<?hh
function Add($x,$y) :mixed{
  return $x+$y;
}
function Sub($x,$y) :mixed{
  return $x-$y;
}
<<__EntryPoint>> function main(): void {
$server = new SoapServer(null,darray['uri'=>"http://testuri.org"]);
$server->addFunction(varray["Sub","Add"]);
var_dump($server->getfunctions());
echo "ok\n";
}
