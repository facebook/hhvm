<?hh

<<__EntryPoint>>
function main() :mixed{
  $deployments = vec["one", "two", "three", "four"];
  $modules = vec["b.c", "b.c.d", "b.d"];

  foreach ($deployments as $deployment) {
    foreach ($modules as $module) {
      $b = __hhvm_intrinsics\is_module_in_deployment($module, $deployment);
      echo "Module $module is " . ($b ? "" : "NOT ") . "in Deployment $deployment\n";
    }
  }
}
