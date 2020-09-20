<?hh <<__EntryPoint>> function main(): void {
$fp = fopen("php://input", "r");
libxml_set_streams_context($fp);
try { libxml_set_streams_context("a"); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "okey";
}
