<?hh <<__EntryPoint>> function main(): void {
$zip = zip_open(dirname(__FILE__)."/test_procedural.zip");
if (!is_resource($zip)) exit("Failure");
$entries = 0;
while ($entry = zip_read($zip)) {
  $entries++;
}
zip_close($zip);
echo "$entries entries";
}
