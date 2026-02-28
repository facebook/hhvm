<?hh <<__EntryPoint>> function main(): void {
date_default_timezone_set('GMT');
echo gmdate('Y-m-d H:i:s', strtotime("2002-06-25 14:18:48.543728"))."\n";
echo gmdate('Y-m-d H:i:s', strtotime("2002-06-25 14:18:48.543728 GMT"))."\n";
echo gmdate('Y-m-d H:i:s', strtotime("2002-06-25 14:18:48.543728 EDT"))."\n";
echo gmdate('Y-m-d H:i:s', strtotime("2002-06-25 14:18:48.543728-00"))."\n";
echo gmdate('Y-m-d H:i:s', strtotime("2002-06-25 14:18:48.543728+00"))."\n";
echo gmdate('Y-m-d H:i:s', strtotime("2002-06-25 14:18:48.543728-04"))."\n";
echo gmdate('Y-m-d H:i:s', strtotime("2002-06-25 14:18:48.543728+04"))."\n";
echo gmdate('Y-m-d H:i:s', strtotime("2002-06-25 14:18:48.543728-0300"))."\n";
echo gmdate('Y-m-d H:i:s', strtotime("2002-06-25 14:18:48.543728+0300"))."\n";
echo gmdate('Y-m-d H:i:s', strtotime("2002-06-25 14:18:48.543728-0330"))."\n";
echo gmdate('Y-m-d H:i:s', strtotime("2002-06-25 14:18:48.543728+0330"))."\n";
}
