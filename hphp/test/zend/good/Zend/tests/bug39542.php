<?hh

function __autoload($class) {
    if (!require_once($class.'.php')) {
        error_log('Error: Autoload class: '.$class.' not found!');
    }
}

<<__EntryPoint>> function main(): void {
$oldcwd = getcwd();
chdir(dirname(__FILE__));
if (substr(PHP_OS, 0, 3) == 'WIN') {
    set_include_path(dirname(__FILE__).'/bug39542;.');
} else {
    set_include_path(dirname(__FILE__).'/bug39542:.');
}

new bug39542();

chdir($oldcwd);
}
