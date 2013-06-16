<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

VERIFY(gethostname());

VS(gethostbyaddr("127.0.0.1"), "localhost.localdomain");
VS(gethostbyname("localhost"), "127.0.0.1");
VS(gethostbynamel("localhost"), array("127.0.0.1"));
VS(getprotobyname("tcp"), 6);
VS(getprotobynumber(6), "tcp");
VS(getservbyname("http", "tcp"), 80);
VS(getservbyport(80, "tcp"), "http");
$packed = chr(127) . chr(0) . chr(0) . chr(1);
VS(inet_ntop($packed), "127.0.0.1");

$packed = str_repeat(chr(0), 15) . chr(1);
VS(inet_ntop($packed), "::1");

$packed = chr(127) . chr(0) . chr(0) . chr(1);
VS(inet_pton("127.0.0.1"), $packed);

$packed = str_repeat(chr(0), 15) . chr(1);
VS(inet_pton("::1"), $packed);
VS(inet_pton("::1"), hex2bin("00000000000000000000000000000001"));

VS(ip2long("127.0.0.1"), 2130706433);
VS(long2ip(2130706433), "127.0.0.1");

VERIFY(dns_check_record("facebook.com"));
VERIFY(checkdnsrr("facebook.com"));

$x = dns_get_record("facebook.com", DNS_A);
VERIFY(!empty($x));

VERIFY(dns_get_mx("facebook.com", $hosts));
VERIFY(!empty($hosts));

VERIFY(getmxrr("facebook.com", $hosts));
VERIFY(!empty($hosts));

$f = fsockopen("facebook.com", 80);
VERIFY($f);
fputs($f, "GET / HTTP/1.0\n\n");
$r = fread($f, 15);
VERIFY(!empty($r));

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
VERIFY(!empty($response));

VS(socket_get_status(new stdclass), false);

$f = fsockopen("facebook.com", 80);
VERIFY($f);
socket_set_blocking($f, 0);

$f = fsockopen("facebook.com", 80);
VERIFY($f);
socket_set_timeout($f, 0);

header("Location: http://www.facebook.com");
header("Location: http://www.facebook.com");
VS(headers_list(), null);

header("Location: http://www.facebook.com");
VERIFY(!headers_sent());

header_remove("name");
header_remove();

VERIFY(!setcookie("name", "value"));
VERIFY(!setcookie("name", "value", 253402300800));

VERIFY(!setrawcookie("name", "value"));

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
