<?hh
<<__EntryPoint>> function main(): void {
$doc=new DOMDocument();
$doc->loadHTML("<tag-throw></tag-throw>",LIBXML_NOERROR);
}
