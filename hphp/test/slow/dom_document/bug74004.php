<?hh
<<__EntryPoint>> function main() {
$doc=new DOMDocument();
$doc->loadHTML("<tag-throw></tag-throw>",LIBXML_NOERROR);
}
