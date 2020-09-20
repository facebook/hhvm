<?hh
/* Prototype         : resource curl_multi_select($mh, $timeout=1.0])
 * Description       : Get all the sockets associated with the cURL extension, which can then be
 *                     "selected"
 * Source code       : ?
 * Test documentation: http://wiki.php.net/qa/temp/ext/curl
 */

<<__EntryPoint>> function main(): void {
//create the multiple cURL handle
$mh = curl_multi_init();
echo curl_multi_select($mh)."\n";

curl_multi_close($mh);
echo "===DONE===\n";
}
