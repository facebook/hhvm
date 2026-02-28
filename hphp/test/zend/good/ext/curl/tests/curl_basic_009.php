<?hh
<<__EntryPoint>> function main(): void {
$url = chr(rand(97,122)).uniqid()."://www.".uniqid().".".uniqid();
$ch = curl_init();
curl_setopt($ch, CURLOPT_URL, $url);

curl_exec($ch);
var_dump(curl_error($ch));
var_dump(curl_errno($ch));
curl_close($ch);
}
