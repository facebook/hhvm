<?php
$test = "
<table>
	<tr><td>first cell before < first cell after</td></tr>
	<tr><td>second cell before < second cell after</td></tr>
</table>";

	var_dump(strip_tags($test));
?>