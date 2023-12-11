<?hh

class hello_world {
  public function hello($to) :mixed{
    return 'Hello ' . $to;
  }
  private function bye($to) :mixed{
    return 'Bye ' . $to;
  }
}
<<__EntryPoint>> function main(): void {
$server = new SoapServer(NULL, dict["uri"=>"test://"]);
$server->setClass('hello_world');
$functions = $server->getfunctions();
foreach($functions as $func) {
  echo $func . "\n";
}
}
