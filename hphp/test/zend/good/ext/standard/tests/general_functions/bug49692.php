<?hh

<<__EntryPoint>>
function main(): void {
  var_dump(parse_ini_file('bug49692.ini', true));
}
