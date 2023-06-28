<?hh

function parseTagsRecursive($input)
:mixed{

    $regex = '#\[indent]((?:[^[]|\[(?!/?indent])|(?R))+)\[/indent]#';

    if (is_array($input)) {
        $input = '<div style="margin-left: 10px">'.$input[1].'</div>';
    }

    $count = -1;
    return preg_replace_callback($regex, parseTagsRecursive<>, $input, -1, inout $count);
}
<<__EntryPoint>>
function main_entry(): void {
  $input = "plain [indent] deep [indent] [abcd]deeper[/abcd] [/indent] deep [/indent] plain";

  $output = parseTagsRecursive($input);

  echo $output, "\n";
}
