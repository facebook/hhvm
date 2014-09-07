<?php
$a = gmp_init("0x41682179fbf5");
printf("Decimal: %s, -36-based: %s\n", gmp_strval($a), gmp_strval($a,-36));
printf("Decimal: %s, 36-based: %s\n", gmp_strval($a), gmp_strval($a,36));
printf("Decimal: %s, -1-based: %s\n", gmp_strval($a), gmp_strval($a,-1));
printf("Decimal: %s, 1-based: %s\n", gmp_strval($a), gmp_strval($a,1));
printf("Decimal: %s, -37-based: %s\n", gmp_strval($a), gmp_strval($a,-37));
printf("Decimal: %s, 37-based: %s\n", gmp_strval($a), gmp_strval($a,37));
printf("Decimal: %s, 62-based: %s\n", gmp_strval($a), gmp_strval($a,62));
printf("Decimal: %s, 63-based: %s\n\n", gmp_strval($a), gmp_strval($a,63));
printf("Base 32 and 62-based: %s\n", gmp_strval(gmp_init("gh82179fbf5", 32), 62));
?>
