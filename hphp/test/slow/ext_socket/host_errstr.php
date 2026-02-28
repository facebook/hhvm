<?hh


<<__EntryPoint>>
function main_host_errstr() :mixed{
$valid = Set {
  'Unknown host', // preferred
  'Host lookup error 1', // shown if system does not have hstrerror
};

var_dump($valid->contains(socket_strerror(-10001)));
}
