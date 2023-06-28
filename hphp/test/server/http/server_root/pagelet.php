<?hh

<<__EntryPoint>>
function pagelet_entrypoint() :mixed{
ob_start();
echo "one";
ob_flush();
pagelet_server_flush();

echo "two";
pagelet_server_flush();
}
