<?hh <<__EntryPoint>> function main(): void {
$zip = zip_open(dirname(__FILE__)."/test_procedural.zip");
if (!is_resource($zip)) exit("Failure");
zip_close($zip);
echo "OK";
}
