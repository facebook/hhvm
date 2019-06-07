<?hh <<__EntryPoint>> function main() {
$ch = curl_init();
curl_close($ch);
var_dump($ch);
echo "===DONE===\n";
}
