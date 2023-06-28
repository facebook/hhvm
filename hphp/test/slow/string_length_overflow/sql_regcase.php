<?hh

<<__EntryPoint>>
function main_sql_regcase() :mixed{
sql_regcase(str_repeat('x',1<<30));
print "Done\n";
}
