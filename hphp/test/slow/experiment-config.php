<?hh

<<__EntryPoint>>
function main() {
  $exps = HH\active_config_experiments();
  $non_exps = HH\inactive_config_experiments();
  $settings = ini_get('hhvm.custom_settings');
  sort(inout $exps);
  sort(inout $non_exps);
  ksort(inout $settings);
  var_dump($exps);
  var_dump($non_exps);
  var_dump($settings);
}
