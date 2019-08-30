<?hh
<<__EntryPoint>> function main(): void {
$url = "http://www.payp\xD0\xB0l.com";

$issues = 0;
$x = new Spoofchecker();
echo "paypal with Cyrillic spoof characters\n";
var_dump($x->isSuspicious($url, inout $issues));

echo "certain all-uppercase Latin sequences can be spoof of Greek\n";
var_dump($x->isSuspicious("NAPKIN PEZ", inout $issues));
var_dump($x->isSuspicious("napkin pez", inout $issues));
}
