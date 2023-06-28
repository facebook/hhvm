<?hh

<<__EntryPoint>>
function subdoc_subdir_test_entrypoint() :mixed{
  echo "DocumentRoot  : {$_SERVER['DOCUMENT_ROOT']}\n";
  echo "ScriptFileName: {$_SERVER['SCRIPT_FILENAME']}\n";
  echo "ScriptName    : {$_SERVER['SCRIPT_NAME']}\n";
}
