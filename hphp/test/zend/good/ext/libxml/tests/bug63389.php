<?hh <<__EntryPoint>> function main(): void {
$fp = fopen("php://input", "r");
libxml_set_streams_context($fp);
echo "okey";
}
