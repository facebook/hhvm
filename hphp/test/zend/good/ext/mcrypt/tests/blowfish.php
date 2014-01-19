<?php
if(!function_exists("hex2bin")) {
    function hex2bin($data) {
       $len = strlen($data);
       return pack("H" . $len, $data);
    }   
}

print "key               plain             crypt             guess             stat\n";
$null = "\0\0\0\0\0\0\0\0";
$vectors = file(dirname(__FILE__) . "/vectors.txt");

$td = mcrypt_module_open ("blowfish", "", MCRYPT_MODE_ECB, "");

foreach($vectors as $data) {
    $data = trim($data);
    if ($data) {
        list($key,$plain,$crypt) = preg_split("/[[:space:]]+/",$data);
        printf("%s  %s  ",
            $key,
            $plain
        );  
        $key = hex2bin(trim($key));
        $plain = hex2bin(($plain));
        $crypt = strtolower(trim($crypt));

        mcrypt_generic_init ($td, $key, $null);
        $guess = mcrypt_generic ($td, $plain);
        $guess = bin2hex($guess);
        printf("%s  %s  %s\n",
            $crypt,
            $guess,
            ($crypt==$guess ? "OK" : "BAD")
        );  
    }   
}

// Longer test case from http://www.schneier.com/code/vectors.txt
$td = mcrypt_module_open ("blowfish", "", MCRYPT_MODE_CBC, "");

$key = hex2bin( "0123456789ABCDEFF0E1D2C3B4A59687" );
$iv = hex2bin( "FEDCBA9876543210" );
$plain = hex2bin( "37363534333231204E6F77206973207468652074696D6520666F722000" );

mcrypt_generic_init( $td, $key, $iv );
$guess = bin2hex( mcrypt_generic( $td, $plain ) );

echo "\n", $guess, "\n";
?>