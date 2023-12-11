<?hh
<<__EntryPoint>> function main(): void {
$tests = vec['"A JSON payload should be an object or array, not a string."',
               '["Unclosed array"',
               '{unquoted_key: "keys must be quoted}',
               '["extra comma",]',
               '["double extra comma",,]',
               '[   , "<-- missing value"]',
               '["Comma after the close"],',
               '["Extra close"]]',
               '{"Extra comma": true,}',
               '{"Extra value after close": true} "misplaced quoted value"',
               '{"Illegal expression": 1 + 2}',
               '{"Illegal invocation": alert()}',
               '{"Numbers cannot have leading zeroes": 013}',
               '{"Numbers cannot be hex": 0x14}',
               '["Illegal backslash escape: \\x15"]',
               '["Illegal backslash escape: \\\'"]',
               '["Illegal backslash escape: \\017"]',
               '[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["Too deep"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]',
               '{"Missing colon" null}',
               '{"Double colon":: null}',
               '{"Comma instead of colon", null}',
               '["Colon instead of comma": false]',
               '["Bad value", truth]',
               "['single quote']"];

foreach ($tests as $test)
{
    echo 'Testing: ' . $test . "\n";
    echo "AS OBJECT\n";
    var_dump(json_decode($test));
    echo "AS ARRAY\n";
    var_dump(json_decode($test, true));
}
}
