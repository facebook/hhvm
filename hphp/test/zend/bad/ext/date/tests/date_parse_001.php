<?php
    date_default_timezone_set('UTC');
    var_dump(date_parse("2006-12-12 10:00:00.5"));
    var_dump(date_parse("2006-12-12"));
    var_dump(date_parse("2006-12--12"));
    var_dump(date_parse("2006-02-30"));
    var_dump(date_parse("2006-03-04"));
    var_dump(date_parse("2006-03"));
    var_dump(date_parse("03-03"));
    var_dump(date_parse("0-0"));
    var_dump(date_parse(""));
    var_dump(date_parse(array()));
    echo "Done\n";
?>