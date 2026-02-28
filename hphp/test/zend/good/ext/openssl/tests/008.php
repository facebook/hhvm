<?hh <<__EntryPoint>> function main(): void {
$fp = fopen(dirname(__FILE__) . "/cert.crt","r");
$a = fread($fp,8192);
fclose($fp);

$b = "file://" . dirname(__FILE__) . "/cert.crt";
$c = "invalid cert";
$d = openssl_x509_read($a);
$e = vec[];

$output = null;
$output2 = null;
$output3 = null;
$output4 = null;
$output5 = null;
var_dump(openssl_x509_export($a, inout $output));    // read cert as a binary string
var_dump(openssl_x509_export($b, inout $output2));    // read cert from a filename string
var_dump(openssl_x509_export($c, inout $output3));    // read an invalid cert, fails
var_dump(openssl_x509_export($d, inout $output4));    // read cert from a resource
var_dump(openssl_x509_export($e, inout $output5));    // read an array, fails

$outfilename = tempnam(sys_get_temp_dir(), "ssl");
if ($outfilename === false)
        exit("failed to get a temporary filename!");

echo "---\n";

var_dump(openssl_x509_export_to_file($a, $outfilename));      // read cert as a binary string
var_dump(openssl_x509_export_to_file($b, $outfilename));      // read cert from a filename string
var_dump(openssl_x509_export_to_file($c, $outfilename));      // read an invalid cert, fails
var_dump(openssl_x509_export_to_file($d, $outfilename));      // read cert from a resource
var_dump(openssl_x509_export_to_file($e, $outfilename));      // read an array, fails
echo "---\n";

var_dump($exists = file_exists($outfilename));
if ($exists) {
        unlink($outfilename);
}
echo "---\n";

var_dump(strcmp($output, $a));
var_dump(strcmp($output, $output2));
var_dump(strcmp($output, (string)$output3));
var_dump(strcmp($output, $output4));    // different
var_dump(strcmp($output, (string)$output5));    // different
}
