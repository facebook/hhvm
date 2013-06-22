<?php

//////////////////////////////////////////////////////////////////////

var_dump(gethostname() != false);

var_dump(strpos(gethostbyaddr("127.0.0.1"), 'localhost'));
var_dump(gethostbyname("localhost"));
var_dump(gethostbynamel("localhost")[0]);
var_dump(getprotobyname("tcp"));
var_dump(getprotobynumber(6));
var_dump(getservbyname("http", "tcp"));
var_dump(getservbyport(80, "tcp"));
$packed = chr(127) . chr(0) . chr(0) . chr(1);
var_dump(inet_ntop($packed));

$packed = str_repeat(chr(0), 15) . chr(1);
var_dump(inet_ntop($packed));

$packed = chr(127) . chr(0) . chr(0) . chr(1);
var_dump(inet_pton("127.0.0.1"));

$packed = str_repeat(chr(0), 15) . chr(1);
var_dump(inet_pton("::1"));
var_dump(inet_pton("::1"));

var_dump(ip2long("127.0.0.1"));
var_dump(long2ip(2130706433));

var_dump(dns_check_record("facebook.com"));
var_dump(checkdnsrr("facebook.com"));

$x = dns_get_record("facebook.com", DNS_A);
var_dump(!empty($x));

var_dump(dns_get_mx("facebook.com", $hosts));
var_dump(!empty($hosts));

var_dump(getmxrr("facebook.com", $hosts));
var_dump(!empty($hosts));

$f = fsockopen("facebook.com", 80);
var_dump($f);
fputs($f, "GET / HTTP/1.0\n\n");
$r = fread($f, 15);
var_dump(!empty($r));

$f = fsockopen("ssl://www.facebook.com", 443);
fwrite($f,
         "GET / HTTP/1.1\r\n".
         "Host: www.facebook.com\r\n".
         "Connection: Close\r\n".
         "\r\n");
$response = '';
while (!feof($f)) {
  $line = fgets($f, 128);
  $response .= $line;
}
var_dump(!empty($response));

var_dump(socket_get_status(new stdclass));

$f = fsockopen("facebook.com", 80);
var_dump($f);
socket_set_blocking($f, 0);

$f = fsockopen("facebook.com", 80);
var_dump($f);
socket_set_timeout($f, 0);

header("Location: http://www.facebook.com");
header("Location: http://www.facebook.com");
var_dump(headers_list());

header("Location: http://www.facebook.com");
var_dump(!headers_sent());

header_remove("name");
header_remove();

var_dump(!setcookie("name", "value"));
var_dump(!setcookie("name", "value", 253402300800));

var_dump(!setrawcookie("name", "value"));

define_syslog_variables();

openlog("TestExtNetwork", LOG_ODELAY, LOG_USER);
syslog(LOG_INFO, "testing");
closelog();

openlog("TestExtNetwork", LOG_ODELAY, LOG_USER);
syslog(LOG_INFO, "testing");
closelog();

openlog("TestExtNetwork", LOG_ODELAY, LOG_USER);
syslog(LOG_INFO, "testing");
closelog();
