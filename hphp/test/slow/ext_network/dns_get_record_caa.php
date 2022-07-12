<?hh

<<__EntryPoint>>
function main_dns_get_record_caa() {
  $_authns = null;
  $_addtl = null;
  // https://caatestsuite.com/ is a formal test suite meant for CAAs and
  // others to validate their CAA parsing / handling logic. Google.com
  // is a prominent company that isn't using Meta infrastructure and which
  // has CAA records. These domains can be replaced with equivalent domains
  // in the event they stop publishing CAA records for some reason.
  $domains = vec['big.basic.caatestsuite.com', 'google.com'];
  foreach ($domains as $domain) {
    $match = false;
    $dns = dns_get_record($domain, DNS_CAA, inout $_authns, inout $_addtl);
    if (count($dns) > 0) {
      if (
        array_key_exists('type', $dns[0]) &&
        $dns[0]['type'] == 'CAA' &&
        array_key_exists('flags', $dns[0]) &&
        array_key_exists('tag', $dns[0]) &&
        array_key_exists('value', $dns[0])
      ) {
        $chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-.";
        if (strlen($dns[0]["value"]) == strspn($dns[0]["value"], $chars)) {
          $match = true;
        }
      }
    }
    if ($match) {
      echo "CAA record found\n";
    } else {
      echo "CAA Lookup failed\n";
      var_dump($dns);
    }
  }
}
