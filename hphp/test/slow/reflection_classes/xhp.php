<?hh

class :div implements XHPChild {}

function foo(:div $arg): :div {
  return $arg;
}

class xhp_prefixed_class {}

function bar(xhp_prefixed_class $arg): xhp_prefixed_class {
  return $arg;
}

function reflect($name) {
  echo '==== ', $name, ' ====', "\n";
  $rf = new ReflectionFunction($name);
  var_dump($rf->getReturnTypeText());
  $params = $rf->getParameters();
  var_dump(count($params));
  var_dump($params[0]->getTypeText());
  var_dump($params[0]->getTypehintText());
}

function main() {
  reflect('foo');
  reflect('bar');
}
main();
