<?php
class foo {
	public    $a="a";
	private   $b="b";
	protected $c="c";
		
}

$x = new SoapClient(NULL,array("location"=>"test://",
                               "uri" => "test://",
                               "exceptions" => 0,
                               "trace" => 1 ));
$x->test(new foo());
echo $x->__getLastRequest();
echo "ok\n";
?>