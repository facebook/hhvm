<?hh

function print_key_status(string $key): void {
  if (apc_exists($key)) {
    echo "$key is in APC\n";
  } else {
    echo "$key is not in APC\n";
  }
}

<<__EntryPoint>>
function main_apc_extend_ttl() {
  apc_store('key_to_extend', 123, 3);
  apc_store('key_to_not_extend', 234, 3);
  apc_store('key_to_extend_later', 345, 3);
  if (!apc_extend_ttl('key_to_extend', 5)) {
    echo "Failed to extend a ttl immediately\n";
  }
  if (apc_extend_ttl('key_to_not_extend', 1)) {
    echo "apc_extend_ttl managed to shorten a ttl\n";
  }
  sleep(2);
  if (!apc_extend_ttl('key_to_extend_later', 3)) {
    echo "Failed to extend a ttl after 2 seconds\n";
  }
  sleep(2);
  print_key_status('key_to_extend');
  print_key_status('key_to_not_extend');
  print_key_status('key_to_extend_later');
  if (!apc_extend_ttl('key_to_extend', 3)) {
    echo "Should be able to extend by shorter ttl later\n";
  }

  if (apc_extend_ttl('foobar123', 10)) {
    echo "Cannot extend nonexistent key\n";
  }
  if (apc_extend_ttl('key_to_not-extend', 10)) {
    echo "Cannot extend expired key\n";
  }
  apc_store('infinite_ttl_key', 111);
  if (apc_extend_ttl('infinite_ttl_key', 1000)) {
    echo "Cannot extend infinite ttl key\n";
  }
  if (apc_extend_ttl('infinite_ttl_key', 0)) {
    echo "Cannot extend infinite ttl key\n";
  }
  if (!apc_extend_ttl('key_to_extend', 0)) {
    echo "Should be allowed to make a key infinite ttl\n";
  }
  if (apc_extend_ttl('key_to_extend', 0)) {
    echo "Should have already been infinite TTL\n";
  }
}
