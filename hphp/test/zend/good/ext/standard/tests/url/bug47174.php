<?hh <<__EntryPoint>> function main(): void {
if (base64_decode("dGVzdA==") == base64_decode("dGVzdA==CRAP")) {
    echo "Same octect data - Signature Valid\n";
} else {
    echo "Invalid Signature\n";
}

$in = base64_encode("foo") . '==' . base64_encode("bar");
var_dump($in, base64_decode($in));
}
