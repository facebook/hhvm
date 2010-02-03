<?php
	$dates = array (
		"1999-10-13",
		"Oct 13  1999",
		"2000-01-19",
		"Jan 19  2000",
		"2001-12-21",
		"Dec 21  2001",
		"2001-12-21 12:16",
		"Dec 21 2001 12:16",
		"Dec 21  12:16",
	    "2001-10-22 21:19:58",
	    "2001-10-22 21:19:58-02",
	    "2001-10-22 21:19:58-0213",
	    "2001-10-22 21:19:58+02",
    	"2001-10-22 21:19:58+0213",
		"2001-10-22T21:20:58-03:40",
		"2001-10-22T211958-2",
		"20011022T211958+0213",
		"20011022T21:20+0215",
		"1997W011",
		"2004W101T05:00+0",
	);

	echo "*** GMT0\n";
///	putenv ("TZ=GMT0");
	foreach ($dates as $date) {
	    echo date ("Y-m-d H:i:s\n", strtotime ($date));
	}

	echo "*** US/Eastern\n";
///	putenv("TZ=US/Eastern");
	if( date("T") == "GMT" ) {
		// POSIX style
///		putenv ("TZ=EST5EDT4,M4.1.0,M10.5.0");
	}

	foreach ($dates as $date) {
	    echo date ("Y-m-d H:i:s\n", strtotime ($date));
	}
