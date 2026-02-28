<?hh


<<__EntryPoint>>
function main_cdata() :mixed{
$node = simplexml_load_string(<<<EOF
<root>
  <hello><![CDATA[world!]]></hello>
</root>
EOF
);

echo $node->hello . "\n";
}
