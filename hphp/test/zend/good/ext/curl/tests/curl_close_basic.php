<?hh <<__EntryPoint>> function main(): void {
$ch = curl_init();
curl_close($ch);
var_dump($ch);
echo "===DONE===\n";
}
