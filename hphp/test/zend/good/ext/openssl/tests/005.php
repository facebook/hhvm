<?hh
<<__EntryPoint>> function main(): void {
$csr = file_get_contents(dirname(__FILE__) . '/005_crt.txt');
if ($out = openssl_csr_get_subject($csr, true)) {
	var_dump($out);
}
echo "\n";
$cn = utf8_decode($out['CN']);
var_dump($cn);
}
