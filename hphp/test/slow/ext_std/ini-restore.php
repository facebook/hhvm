<?hh


<<__EntryPoint>>
function main_ini_restore() {
ini_set('zend.enable_gc', false);
ini_restore('zend.enable_gc');
}
