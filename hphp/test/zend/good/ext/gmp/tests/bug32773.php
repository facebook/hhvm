<?php
echo '10 + 0 = ', gmp_strval(gmp_add(10, 0)), "\n";
echo '10 + "0" = ', gmp_strval(gmp_add(10, '0')), "\n";
                                                                                                              
echo gmp_strval(gmp_div(10, 0))."\n";
echo gmp_strval(gmp_div_qr(10, 0))."\n";
                                                                                                              
?>
