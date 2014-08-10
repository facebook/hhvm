<?php
$client = new SoapClient(null, array('uri' => 'mo:http://www.w3.org/', 'location' => 'http://some.url'));
$client->__setCookie("CookieTest", "HelloWorld");
var_dump($client->__getCookies()['CookieTest'][0]);
?>
