<?hh

<<__EntryPoint>>
function main() :mixed{
  $deployments = vec["one", "two", "three", "four"];
  $modules = vec["a.b", "c.e", "y.z", "b.d"];

  foreach ($deployments as $deployment) {
    foreach ($modules as $module) {
      $b = __hhvm_intrinsics\is_module_in_deployment($module, $deployment);
      if ($b) {
        echo "Module $module is in Deployment $deployment\n";
      }
    }
  }
}
