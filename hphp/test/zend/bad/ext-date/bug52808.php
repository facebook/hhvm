<?php
date_default_timezone_set('Europe/Oslo');
$intervals = array(
	"2008-05-11T15:30:00Z/2007-03-01T13:00:00Z",
	"2007-05-11T15:30:00Z/2008-03-01T13:00:00Z",
	"2007-05-11T15:30:00Z 2008-03-01T13:00:00Z",
	"2007-05-11T15:30:00Z/",
	"2007-05-11T15:30:00Z",
	"2007-05-11T15:30:00Z/:00Z",
);
foreach($intervals as $iv) {
    try
    {
    	$di = new DateInterval($iv);
    	var_dump($di);
    }
    catch ( Exception $e )
    {
    	echo $e->getMessage(), "\n";
    }
}
echo "==DONE==\n";
?>