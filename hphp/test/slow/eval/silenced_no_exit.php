<?hh


<<__EntryPoint>>
function main_silenced_no_exit() :mixed{
echo "Before error, there should be an 'After error'\n";
@eval("invalid");
echo "After error\n";
}
