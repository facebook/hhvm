<?hh
<<__EntryPoint>> function main(): void {
$doc = new DOMDocument();
$doc->load(dirname(__FILE__)."/nsdoc.xml");

$root = $doc->documentElement;

$duri = $doc->lookupNamespaceURI("ns2")."\n";
$euri = $root->lookupNamespaceURI("ns2")."\n";

var_dump($duri == $euri);

$dpref = $doc->lookupPrefix("http://ns2")."\n";
$epref = $root->lookupPrefix("http://ns2")."\n";

var_dump($dpref == $epref);

$disdef = $doc->isDefaultNamespace("http://ns")."\n";
$eisdef = $root->isDefaultNamespace("http://ns")."\n";

var_dump($dpref === $epref);
}
