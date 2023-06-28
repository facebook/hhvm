<?hh


<<__EntryPoint>>
function main_finfo_unserialize() :mixed{
$f = new finfo(FILEINFO_MIME_TYPE);
echo unserialize(serialize($f))->file(__FILE__), "\n";
$f->set_flags(FILEINFO_NONE);
echo unserialize(serialize($f))->file(__FILE__), "\n";
}
