<?hh


<<__EntryPoint>>
function main_1372() :mixed{
register_shutdown_function(function() {
  $error = error_get_last();

  if($error['type'] & E_ERROR) {
    echo "Error\n";
  }
});

Foo::bar();
}
