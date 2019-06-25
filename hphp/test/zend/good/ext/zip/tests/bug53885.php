<?hh <<__EntryPoint>> function main(): void {
$fname = dirname(__FILE__)."/test53885.zip";
if(file_exists($fname)) unlink($fname);
touch($fname);
$nx=new ZipArchive();
$nx->open($fname);
$nx->locateName("a",ZIPARCHIVE::FL_UNCHANGED);
$nx->statName("a",ZIPARCHIVE::FL_UNCHANGED);

echo "==DONE==\n";
error_reporting(0);
$fname = dirname(__FILE__)."/test53885.zip";
unlink($fname);
}
