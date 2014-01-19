<?php

class hello_world {   
  public function hello($to) {
    return 'Hello ' . $to;
  }    
  private function bye($to) {
    return 'Bye ' . $to;
  }    
}

$server = new SoapServer(NULL, array("uri"=>"test://"));
$server->setClass('hello_world');
$functions = $server->getFunctions();
foreach($functions as $func) {
  echo $func . "\n";
}
?>