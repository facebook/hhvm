<?hh
<<__EntryPoint>> function main(): void {
$file = __SystemLib\hphp_test_tmppath('bug44461.ini');
file_put_contents($file, <<<EOF
[attachments]
zip = "application/zip" ; MIME-type for ZIP files
EOF
);
parse_ini_file($file, true);
echo "===DONE===\n";

unlink($file);
}
