<?hh

show(getallheaders());
show(HH\get_headers_secure());

function show($a) {
  ksort(&$a);
  var_dump($a);
}
