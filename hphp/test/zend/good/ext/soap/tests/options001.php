<?hh
class TestSoapClient extends SoapClient{}

<<__EntryPoint>> function main(): void {
  $options=dict[
    'actor' =>'http://schemas.nothing.com',
    'proxy_host' => 'myproxy',
    'proxy_port' => 80,
    'proxy_ssl_cert_path' => 'test',
    'proxy_ssl_key_path' => 'test',
    'proxy_ssl_ca_bundle' => 'test'
  ];

  $client = new TestSoapClient(dirname(__FILE__)."/classmap.wsdl",$options);
  var_dump($client);
  echo "ok\n";
}
