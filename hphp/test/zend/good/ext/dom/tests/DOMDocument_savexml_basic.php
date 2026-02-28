<?hh
<<__EntryPoint>> function main(): void {
$xml = <<< EOXML
<?xml version="1.0" encoding="utf-8"?>
<courses>
	<!-- Hello World! -->
	<aNode>
		<childNode>
			<childlessNode />
		</childNode>
	</aNode>
</courses>
EOXML;

$dom = new DOMDocument();
$dom->loadXML($xml);
$root = $dom->documentElement;

$filename = sys_get_temp_dir().'/'.'tmp_dom_savexml';
var_dump($dom->save($filename));
$result = file_get_contents($filename);
var_dump($result == $dom->saveXML());

unlink($filename);
}
