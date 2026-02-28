<?hh 

class SXETest extends SimpleXMLIterator
{
	function count()
:mixed	{
		echo __METHOD__ . "\n";
		return parent::count();
	}
}
<<__EntryPoint>>
function entrypoint_sxe_005(): void {

  $xml =<<<EOF
<?xml version='1.0'?>
<sxe>
  <elem1/>
  <elem2/>
  <elem2/>
</sxe>
EOF;

  $sxe = new SXETest((string)$xml);

  var_dump(count($sxe));
  var_dump(count($sxe->elem1));
  var_dump(count($sxe->elem2));

  echo "===DONE===\n";
}
