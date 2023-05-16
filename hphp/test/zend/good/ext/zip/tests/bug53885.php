<?hh
<<__EntryPoint>> function main(): void {
$fname = sys_get_temp_dir().'/'.'test53885.zip';
if(file_exists($fname)) unlink($fname);
touch($fname);
$nx=new ZipArchive();
$nx->open($fname);
$nx->locateName("a",ZipArchive::FL_UNCHANGED);
$nx->statName("a",ZipArchive::FL_UNCHANGED);

echo "==DONE==\n";

unlink($fname);
}
