<?php

$client = new SoapClient(null, array('proxy_host'     => "localhost",
                                            'proxy_port'     => '8080',
                                            'login'    => "user",
									        'password' => "test",
											'uri'            => 'mo:http://www.w3.org/',
											'location'       => 'http://some.url'));
var_dump($client->_proxy_port);

?>