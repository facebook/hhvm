<?hh
class foo {
	function __construct($name) {
    $GLOBALS['List']= $this;
    $this->Name = $name;
		$GLOBALS['List']->echoName();
	}

	function echoName() {
    $GLOBALS['names'] ??= varray[];
    $GLOBALS['names'][] = $this->Name;
	}
}


$bar1 =new foo('constructor');
$bar1->Name = 'outside';
$bar1->echoName();
$List->echoName();

print ($names==varray['constructor','outside','outside']) ? 'success':'failure';
