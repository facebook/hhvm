<?hh
<<__EntryPoint>> function main(): void {
chdir(sys_get_temp_dir());
$file = md5('SplFileInfo::getExtension');
$exts = vec['.txt', '.extension', '..', '.', ''];
foreach ($exts as $ext) {
    touch($file . $ext);
    $info = new SplFileInfo($file . $ext);
    var_dump($info->getExtension(), pathinfo($file . $ext, PATHINFO_EXTENSION));
}

foreach ($exts as $ext) {
    unlink($file . $ext);
}
}
