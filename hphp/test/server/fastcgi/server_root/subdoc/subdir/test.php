<?hh

<<__EntryPoint>>
function subdoc_subdir_test_entrypoint() :mixed{
  echo "DocumentRoot  : ".\HH\global_get('_SERVER')['DOCUMENT_ROOT']."\n";
  echo "ScriptFileName: ".\HH\global_get('_SERVER')['SCRIPT_FILENAME']."\n";
  echo "ScriptName    : ".\HH\global_get('_SERVER')['SCRIPT_NAME']."\n";
}
