<?hh

function replace_spaces($text) :mixed{
    $lambda = function ($matches) {
        return str_replace(' ', '&nbsp;', $matches[1]).' ';
    };
    $count = -1;
    return preg_replace_callback('/( +) /', $lambda, $text, -1, inout $count);
}
<<__EntryPoint>> function main(): void {
echo replace_spaces("1 2 3\n");
echo replace_spaces("1  2  3\n");
echo replace_spaces("1   2   3\n");
echo "Done\n";
}
