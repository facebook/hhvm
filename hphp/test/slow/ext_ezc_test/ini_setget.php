<?php

printf("BEGIN\n");

printf("PHP ezc_test.integer %d\n", ini_get("ezc_test.integer")); // 42
printf("PHP ezc_test.string  %s\n", ini_get("ezc_test.string"));  // foobar

printf("SET int\n");
ini_set("ezc_test.integer", 57);
printf("SET string\n");
ini_set("ezc_test.string", "helium");

printf("PHP ezc_test.integer %d\n", ini_get("ezc_test.integer"));  // 57
printf("PHP ezc_test.string  %s\n", ini_get("ezc_test.string"));   // helium

printf("END\n");
