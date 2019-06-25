<?hh
function Add($x,$y) {
  return $x+$y;
}
function Sub($x,$y) {
  return $x-$y;
}
<<__EntryPoint>> function main(): void {
$server = new soapserver(null,array('uri'=>"http://testuri.org"));
$server->addfunction(array("Sub","Add"));
var_dump($server->getfunctions());
echo "ok\n";
}
