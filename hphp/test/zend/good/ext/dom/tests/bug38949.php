<?hh
<<__EntryPoint>> function main(): void {
$doc = new DOMDocument();
$doc->load(dirname(__FILE__)."/nsdoc.xml");

$root = $doc->documentElement;

echo $root->getAttribute("xmlns")."\n";
echo $root->getAttribute("xmlns:ns2")."\n";

$child = $root->firstChild->nextSibling;
echo $child->getAttribute("xmlns")."\n";
echo $child->getAttribute("xmlns:ns2")."\n";

echo "DONE\n";
}
