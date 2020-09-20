<?hh
function Add($x,$y) {
  return $x+$y;
}
function Sub($x,$y) {
  return $x-$y;
}
<<__EntryPoint>> function main(): void {
$server = new soapserver(null,darray['uri'=>"http://testuri.org"]);
$server->addfunction(varray["Sub","Add"]);
var_dump($server->getfunctions());
echo "ok\n";
}
