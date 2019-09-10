<?hh
$input = "plain [indent] deep [indent] [abcd]deeper[/abcd] [/indent] deep [/indent] plain";

function parseTagsRecursive($input)
{

    $regex = '#\[indent]((?:[^[]|\[(?!/?indent])|(?R))+)\[/indent]#';

    if (is_array($input)) {
        $input = '<div style="margin-left: 10px">'.$input[1].'</div>';
    }

    $count = -1;
    return preg_replace_callback($regex, 'parseTagsRecursive', $input, -1, inout $count);
}

$output = parseTagsRecursive($input);

echo $output, "\n";
