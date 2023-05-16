<?hh
<<__EntryPoint>> function main(): void {
$file = sys_get_temp_dir().'/'.'bug44461.ini';
file_put_contents($file, <<<EOF
[attachments]
zip = "application/zip" ; MIME-type for ZIP files
EOF
);
parse_ini_file($file, true);
echo "===DONE===\n";

unlink($file);
}
