<?hh


<<__EntryPoint>>
function main_dns_get_record() :mixed{
  $authns = null;
  $addtl = null;
  var_dump(dns_get_record('127.0.0.1', DNS_A, inout $authns, inout $addtl));
}
