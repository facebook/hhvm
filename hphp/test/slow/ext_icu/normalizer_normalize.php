<?php

// based on example at http://www.php.net/manual/en/normalizer.normalize.php

function main() {
  // 'LATIN CAPITAL LETTER A WITH RING ABOVE' (U+00C5)
  $char_A_ring = "\xC3\x85";
  // 'COMBINING RING ABOVE' (U+030A)
  $char_combining_ring_above = "\xCC\x8A";

  $char_1 = normalizer_normalize($char_A_ring, Normalizer::FORM_C);
  $char_2 = normalizer_normalize(
    'A'.$char_combining_ring_above,
    Normalizer::FORM_C
  );

  var_dump(urlencode($char_1));
  var_dump(urlencode($char_2));
}

main();
