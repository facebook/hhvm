<?php 
	function m($f,$a){
		return array_map($f,0);
	}

	echo implode(m("",m("",m("",m("",m("0000000000000000000000000000000000",("")))))));
?>
