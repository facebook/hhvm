<?hh <<__EntryPoint>> function main(): void {
$prv = 'file://' . dirname(__FILE__) . '/' . 'bug41033.pem';
$pub = 'file://' . dirname(__FILE__) . '/' . 'bug41033pub.pem';


$prkeyid = openssl_get_privatekey($prv, "1234");
$ct = "Hello I am some text!";
$signature = null;
openssl_sign($ct, inout $signature, $prkeyid, OPENSSL_ALGO_SHA1);
echo "Signature: ".base64_encode($signature) . "\n";

$pukeyid = openssl_get_publickey($pub);
$valid = openssl_verify($ct, $signature, $pukeyid, OPENSSL_ALGO_SHA1);
echo "Signature validity: " . $valid . "\n";
}
