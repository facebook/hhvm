<?hh
var_dump(libxml_use_internal_errors(true));
echo "suppressing\n";
libxml_suppress_errors(true);

$xmlstr = <<<XML
<?xml version='1.0' standalone='yes'?>
    <movies>
        <movie>
            <titles>PHP: Behind the Parser</title>
        </movie>
    </movies>
XML;

simplexml_load_string($xmlstr);
var_dump(libxml_get_errors());
// re-enable
echo "unsuppressing\n";
libxml_suppress_errors(false);
simplexml_load_string($xmlstr);
var_dump(libxml_get_errors());

// test memleaks here
var_dump(libxml_use_internal_errors(false));

echo "Done\n";
