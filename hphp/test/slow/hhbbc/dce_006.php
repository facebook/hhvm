<?hh

function foo($params) {
  $listId = (isset($params['id']) && trim($params['id']) != '') ?
    ((int)$params['id'] > 0?(int)$params['id'] :
     ((string)$params['id'] == 'default'?'default': null)):null;
  return $params;
}


<<__EntryPoint>>
function main_dce_006() {
var_dump(foo(darray['id' => '42']));
}
