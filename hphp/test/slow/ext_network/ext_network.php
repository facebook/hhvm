<?php

//////////////////////////////////////////////////////////////////////

function tryopen($u, $p = -1) {
  for ($i = 0; $i < 100; $i++) {
    @$r = $p >= 0 ? fsockopen($u, $p) : fsockopen($u);
    if ($r) return $r;
    usleep(1000);
  }
  return $r;
}

function get_addresses($host) {
  $r = array();
  if (($records = dns_get_record($host))) {
    foreach ($records as $record) {
      if (isset($record['ipv6'])) {
        $r []= '['.$record['ipv6'].']';
      } else if (isset($record['ip'])) {
        $r []= $record['ip'];
      }
    }
  }
  return $r;
}

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
