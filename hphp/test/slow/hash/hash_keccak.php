<?hh


// Test vectors taken from a combination of NIST FIPS-202,
// Wikipedia reference vectors,
// and output from reference implementation

<<__EntryPoint>>
function main_hash_keccak() :mixed{
$subjects = vec[
  '',
  'a',
  'The quick brown fox jumps over the lazy dog',
  'The quick brown fox jumps over the lazy dog.',
  str_repeat('a', 257),
  str_repeat("\xA3", 200),
];

foreach ($subjects as $subject) {
  echo '== ', urlencode($subject), " ==\n";
  foreach (vec[224, 256, 384, 512] as $bits) {
    echo $bits, ': ', hash("sha3-$bits", $subject), "\n";
  }
}
}
