<?php
unlink("1.txt");
$w = fopen("1.txt", "w");
$r = fopen("1.txt", "r");
printf("Read %d bytes\n", strlen(fread($r, 10)));
fwrite($w, "hello\n");fclose($w);
printf("Read %d bytes\n", strlen(fread($r, 10)));
