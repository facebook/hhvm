<?hh
<<__EntryPoint>> function main(): void {
$xml = simplexml_load_string('<root>
	<first_node_no_ns />
	<ns1:node1 xmlns:ns1="#ns1" />
	<ns2:node2 xmlns:ns2="#ns2" />
	<ns3:node3 xmlns:ns3="#ns3" />
	<last_node_no_ns />
</root>');

$name = $xml->getName();
$namespaces = $xml->getNamespaces(True);
echo "root(recursive): '$name' -- namespaces: ", implode(', ', $namespaces), "\n";
$namespaces = $xml->getNamespaces(False);
echo "root(non-recursive): '$name' -- namespaces: ", implode(', ', $namespaces), "\n";

foreach (vec['', '#ns1', '#ns2', '#ns3'] as $ns)
{
	foreach ($xml->children($ns) as $child)
	{
		$name = $child->getName();
		$namespaces = $child->getNamespaces(false);

		echo "children($ns): '$name' -- namespaces: ", implode(', ', $namespaces), "\n";
	}
}
echo "===DONE===\n";
}
