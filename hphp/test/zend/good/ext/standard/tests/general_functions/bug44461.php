<?hh <<__EntryPoint>> function main(): void {
file_put_contents(__DIR__ . 'bug44461.ini', <<<EOF
[attachments]
zip = "application/zip" ; MIME-type for ZIP files
EOF
);
parse_ini_file(__DIR__ . 'bug44461.ini', true);
echo "===DONE===\n";
error_reporting(0);
unlink(__DIR__ . 'bug44461.ini');
}
