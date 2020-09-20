<?hh
<<__EntryPoint>> function main(): void {
chdir(dirname(__FILE__));
setlocale(LC_ALL, 'en_US.UTF-8');
bindtextdomain ("messages", "./locale");
echo textdomain('test'), "\n";
echo textdomain(''), "\n";
echo textdomain('foo'), "\n";
}
