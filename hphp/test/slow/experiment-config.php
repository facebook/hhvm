<?hh

<<__EntryPoint>>
function main() {
  $exps = hh\active_config_experiments();
  $non_exps = hh\inactive_config_experiments();
  $settings = ini_get('hhvm.custom_settings');
  sort(inout $exps);
  sort(inout $non_exps);
  ksort(inout $settings);
  var_dump($exps);
  var_dump($non_exps);
  var_dump($settings);
}
