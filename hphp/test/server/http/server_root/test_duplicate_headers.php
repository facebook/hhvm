<?hh

show(getallheaders());
show(HH\get_headers_secure());

function show($a) {
  ksort(inout $a);
  var_dump($a);
}
