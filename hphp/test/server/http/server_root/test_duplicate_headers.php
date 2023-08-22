<?hh

<<__EntryPoint>>
function test_duplicate_headers_entrypoint() :mixed{
show(HH\get_headers_secure());
}

function show($a) :mixed{
  ksort(inout $a);
  var_dump($a);
}
