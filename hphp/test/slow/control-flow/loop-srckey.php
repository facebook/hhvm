<?hh


function subdomains($domain) {
  $domains = varray[];
  $components = explode('.', strtolower($domain));
  while (count($components) > 0) {
    $domains[] = implode('.', $components);
    array_shift(inout $components);
  }
  return $domains;
}


<<__EntryPoint>>
function main_loop_srckey() {
$domains = array_fill(0, 22, 'www.facebook');
foreach ($domains as $d) {
  var_dump(subdomains($d));
}
}
