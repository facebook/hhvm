<?hh <<__EntryPoint>> function main(): void {
foreach (mcrypt_list_algorithms() as $algo) {
    if (in_array($algo, varray['rijndael-256', 'des', 'blowfish', 'twofish'])) {
       echo "FOUND\n";
    }
}
}
