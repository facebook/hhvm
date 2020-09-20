<?hh <<__EntryPoint>> function main(): void {
$xml = "<?xml version=\"1.0\"?>
<!DOCTYPE note [
<!ELEMENT note (to,from,heading,body)>
<!ELEMENT to (#PCDATA)>
<!ELEMENT from (#PCDATA)>
<!ELEMENT heading (#PCDATA)>
<!ELEMENT body (#PCDATA)>
]>
<note>
<to>Tove</to>
<from>Jani</from>
<heading>Reminder</heading>
<body>Don't forget me this weekend</body>
</note>";
$dom = new DOMDocument('1.0');
$dom->loadXML($xml);
var_dump($dom->validate());
}
