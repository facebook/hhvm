<?hh


<<__EntryPoint>>
function main_import_simlexml() :mixed{
$xml = simplexml_load_string(<<<XML
<top>
    <first>
        <second>second</second>
    </first>
</top>
XML
);

$node = dom_import_simplexml($xml->first->offsetGet(0)->second);
var_dump($node->tagName);
var_dump($node->parentNode->tagName);

$node = dom_import_simplexml($xml->first->offsetGet(0)->second->offsetGet(0));
var_dump($node->tagName);
var_dump($node->parentNode->tagName);
}
