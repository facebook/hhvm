<?hh
<<__EntryPoint>> function main(): void {
chdir(__SystemLib\hphp_test_tmproot());
$file = md5('SplFileInfo::getExtension');
$exts = varray['.txt', '.extension', '..', '.', ''];
foreach ($exts as $ext) {
    touch($file . $ext);
    $info = new SplFileInfo($file . $ext);
    var_dump($info->getExtension(), pathinfo($file . $ext, PATHINFO_EXTENSION));
}

foreach ($exts as $ext) {
    unlink($file . $ext);
}
}
