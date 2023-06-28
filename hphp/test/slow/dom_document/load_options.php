<?hh

// saveHTML() should not add a default doctype when
// LIBXML_HTML_NODEFDTD is passed
<<__EntryPoint>>
function main_load_options() :mixed{
$doc = new DOMDocument();
$doc->loadHTML("<html><body>test</body></html>", LIBXML_HTML_NODEFDTD);
echo $doc->saveHTML();
}
