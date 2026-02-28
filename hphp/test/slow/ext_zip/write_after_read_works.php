<?hh

// Create test ZIP file
<<__EntryPoint>>
function main_write_after_read_works() :mixed{
$tempfile = sys_get_temp_dir().'/'.'hello.zip';
$zip_setup = new ZipArchive();
$zip_setup->open($tempfile, ZipArchive::OVERWRITE | ZipArchive::CREATE);
$zip_setup->addFromString('hello.txt', 'Old value here');
$zip_setup->addFromString('hello2.txt', 'Old value here');
$zip_setup->close();

// Load the file fresh to modify it
$zip = new ZipArchive();
$zip->open($tempfile);

// It works if you comment out this line
echo 'Old value: ', $zip->getFromName('hello.txt'), "\n";
$new = 'It worked at '.time();
$zip->addFromString('hello.txt', $new);
$zip->addFromString('hello2.txt', $new);
echo 'New value: ', $new, "\n";
$zip->close();

// Reload the ZIP to show that it hasn't actually been modified
$zip = new ZipArchive();
$zip->open($tempfile);
echo 'Reloaded value hello.txt: ', $zip->getFromName('hello.txt'), "\n";
echo 'Reloaded value hello2.txt: ', $zip->getFromName('hello2.txt'), "\n";

unlink($tempfile);
}
