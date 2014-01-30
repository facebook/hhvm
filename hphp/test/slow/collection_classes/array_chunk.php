<?hh

function test_chunk($input, $chunk_size) {
  echo '====================', "\n";
  var_dump($input);
  $chunked_without_keys = array_chunk($input, $chunk_size);
  var_dump($chunked_without_keys);
  var_dump($chunked_without_keys === array_chunk((array) $input, $chunk_size));
  $chunked_with_keys = array_chunk($input, $chunk_size, true);
  var_dump($chunked_with_keys);
  var_dump($chunked_with_keys
           === array_chunk((array) $input, $chunk_size, true));
}

function main() {
  test_chunk(Vector {"a", "b", "c", "d", "e"}, 2);
  test_chunk(Set {"a", "b", "c", "d", "e"}, 2);
  test_chunk(Map {1 => "a", 2 => "b", 3 => "c", 4 => "d", 5 => "e"}, 2);
  test_chunk(
    Map {'a' => "a", 'b' => "b", 'c' => "c",
        'd' => "d", 'e' => "e"},
    2
  );
}
main();
