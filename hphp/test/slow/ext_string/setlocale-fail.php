<?php

setlocale(LC_ALL, "");

// Let's try some terrible locales to ensure they fail
test(LC_ALL, 'LC_ALL=fr_FR');
test(LC_ALL, 'LC_ALL=fr_FR;');
test(LC_ALL, 'LC_NUMERIC=fr_FR');
test(LC_ALL, 'LC_NUMERIC=fr_FR;');
test(LC_ALL, 'fr_FR;');
test(LC_ALL, '=fr_FR');
test(LC_ALL, '=fr_FR;');

test(LC_NUMERIC, 'LC_NUMERIC=fr_FR');

// This weirdly is a valid statement for newlocale/uselocale
// but not for setlocale.
// We check for and disable it explicitly in the code.
test(LC_NUMERIC, 'LC_NUMERIC=fr_FR;');

test(LC_NUMERIC, 'fr_FR;');
test(LC_NUMERIC, '=fr_FR');
test(LC_NUMERIC, '=fr_FR;');

function test($category, $locale) {
  var_dump(setlocale($category, $locale));
  echo sprintf("%.3f\n", 3.142);
  setlocale(LC_ALL, "");
}
