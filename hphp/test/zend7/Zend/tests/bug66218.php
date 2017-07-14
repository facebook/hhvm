<?php
$tab = get_extension_funcs("standard");
$fcts = array("dl");
foreach ($fcts as $fct) {
	if (in_array($fct, $tab)) {
		echo "$fct Ok\n";
	}
}
?>
Done
