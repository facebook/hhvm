<?php
date_default_timezone_set('Europe/Oslo');
$formats = array( DATE_ATOM, DATE_COOKIE, DATE_ISO8601, DATE_RFC822,
		DATE_RFC850, DATE_RFC1036, DATE_RFC1123, DATE_RFC2822, DATE_RFC3339,
		DATE_RSS, DATE_W3C );

foreach( $formats as $format )
{
	$date = new DateTime( "2008-07-08T22:14:12+02:00" );
	$formatted = $date->format( $format ) ;
	$date2 = date_create_from_format( $format, $formatted );
	var_dump( $format, $formatted, $date2 );
	echo "\n";
	if ( $date2 === false )
	{
		var_dump(date_parse_from_format( $format, $formatted ) );
	}
}
?>