<?php
date_default_timezone_set('Europe/Oslo');
$sun_info = date_sun_info(strtotime("2007-04-13 08:31:15 UTC"), 59.21, 9.61);
foreach ($sun_info as $key => $elem )
{
	echo date( 'Y-m-d H:i:s T', $elem ),  " ", $key, "\n";
}
echo "Done\n";
?>