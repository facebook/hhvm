<?php
echo "-Test\n";

sscanf("2147483647", '%d', $int);
echo "sscanf 32-bit signed int '2147483647'           (2^31)-1 = ",$int,"\n";
sscanf("4294967295", '%u', $int);
echo "sscanf 32-bit unsign int '4294967295'           (2^32)-1 = ",$int,"\n";

sscanf("9223372036854775807", '%d', $int);
echo "sscanf 64-bit signed int '9223372036854775807'  (2^63)-1 = ",$int,"\n";
sscanf("18446744073709551615", '%u', $int);
echo "sscanf 64-bit unsign int '18446744073709551615' (2^64)-1 = ",$int,"\n";

printf("printf 64-bit signed int '9223372036854775807'  (2^63)-1 = %d\n", 9223372036854775807);
printf("printf 64-bit signed int '18446744073709551615' (2^64)-1 = %u\n", 18446744073709551615);

echo "Done\n";
?>