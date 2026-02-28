<?hh

<<__EntryPoint>>
function main_heredoc_end_after_variable() :mixed{
$var = 'HERE';
$str = <<<DOC
{$var}DOC
DOC;
echo $str;
}
