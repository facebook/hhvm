<?hh

$str = "first=value&arr[]=foo+bar&arr[]=baz";
parse_str($str);
echo $first;
