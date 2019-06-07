<?hh

// Tests for segfault if no connection available

<<__EntryPoint>>
function main_mysql_escape_string_segfault() {
@mysql_real_escape_string("");

echo "ok";
}
