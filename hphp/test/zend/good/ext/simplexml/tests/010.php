<?hh

class simplexml_inherited extends SimpleXMLElement
{
}
<<__EntryPoint>> function main(): void {
$xml =<<<EOF
<?xml version='1.0'?>
<!DOCTYPE sxe SYSTEM "notfound.dtd">
<sxe id="elem1">
 <elem1 attr1='first'>
  <!-- comment -->
  <elem2>
   <elem3>
    <elem4>
     <?test processing instruction ?>
    </elem4>
   </elem3>
  </elem2>
 </elem1>
</sxe>
EOF;

var_dump(simplexml_load_string($xml, 'simplexml_inherited'));

echo "===DONE===\n";
}
