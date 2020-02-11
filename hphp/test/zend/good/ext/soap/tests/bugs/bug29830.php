<?hh

class hello_world {
  public function hello($to) {
    return 'Hello ' . $to;
  }
  private function bye($to) {
    return 'Bye ' . $to;
  }
}
<<__EntryPoint>> function main(): void {
$server = new SoapServer(NULL, darray["uri"=>"test://"]);
$server->setClass('hello_world');
$functions = $server->getFunctions();
foreach($functions as $func) {
  echo $func . "\n";
}
}
