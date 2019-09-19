<?hh
<<__EntryPoint>>
function main_entry(): void {

  require_once('test_base.inc');

  runTest(function ($port) {
      var_dump(http_request(php_uname('n'), $port, 'static_content.txt', 1200, '--head'));
    },
    '-d hhvm.static_file.files_match[0][pattern]=".*" '.
    '-d hhvm.static_file.files_match[0][headers][]="Abc: 1"'
  );
}
