<?php
include "connect.inc";

$link = ldap_connect($host, $port);

$dn = "dc=my-domain,dc=com";
$filter = "(dc=*)";

$result = ldap_search();
var_dump($result);

$result = ldap_search($link, $dn, $filter);
var_dump($result);

$result = ldap_search($link, $dn, $filter, NULL);
var_dump($result);

$result = ldap_search($link, $dn, $filter, array(1 => 'top'));
var_dump($result);

$result = ldap_search(array(), $dn, $filter, array('top'));
var_dump($result);

$result = ldap_search(array($link, $link), array($dn), $filter, array('top'));
var_dump($result);

$result = ldap_search(array($link, $link), $dn, array($filter), array('top'));
var_dump($result);
?>
===DONE===