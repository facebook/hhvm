<?hh

function dump_property($rp) :mixed{
  var_dump($rp);
  // dynamic props should always be public (true, false, false)
  var_dump($rp->isPublic(), $rp->isProtected(), $rp->isPrivate());
  // dynamic props are never static, never declared (aka default), and
  // always ReflectionProperty::IS_PUBLIC (true, false, 256)
  var_dump($rp->isStatic(), $rp->isDefault(), $rp->getModifiers());
  // there's no doc comment, type text, or default value (false, "", null)
  var_dump($rp->getDocComment(), $rp->getTypeText(), $rp->getDefaultValue());
}


<<__EntryPoint>>
function main_dynamic_property() :mixed{
$a = new stdClass;
$a->a = 'a';
dump_property((new ReflectionObject($a))->getProperty('a'));
dump_property((new ReflectionProperty($a, 'a')));
}
