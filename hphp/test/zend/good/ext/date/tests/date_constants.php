<?php
    date_default_timezone_set("Europe/Oslo");
    $constants = array(
        DATE_ATOM,
        DATE_COOKIE,
        DATE_ISO8601,
        DATE_RFC822,
        DATE_RFC850,
        DATE_RFC1036,
        DATE_RFC1123,
        DATE_RFC2822,
        DATE_RFC3339,
        DATE_RSS,
        DATE_W3C
    );
    
    foreach($constants as $const) {
        var_dump(date($const, strtotime("1 Jul 06 14:27:30 +0200")));
        var_dump(date($const, strtotime("2006-05-30T14:32:13+02:00")));
    }

    print "\n";

    var_dump(
        DATE_ATOM    == DateTime::ATOM,
        DATE_COOKIE  == DateTime::COOKIE,
        DATE_ISO8601 == DateTime::ISO8601,
        DATE_RFC822  == DateTime::RFC822,
        DATE_RFC850  == DateTime::RFC850,
        DATE_RFC1036 == DateTime::RFC1036,
        DATE_RFC1123 == DateTime::RFC1123,
        DATE_RFC2822 == DateTime::RFC2822,
        DATE_RFC3339 == DateTime::RFC3339,
        DATE_RSS     == DateTime::RSS,
        DATE_W3C     == DateTime::W3C
    );
?>