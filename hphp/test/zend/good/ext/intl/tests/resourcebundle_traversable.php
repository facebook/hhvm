<?hh

<<__EntryPoint>>
function main_entry(): void {
    include "resourcebundle.inc";

  	$r = new ResourceBundle( 'es', $bundle );

  	// This is actually HH\Traversable due to autoimport
  	var_dump($r is Traversable);
  	var_dump(iterator_to_array($r->get('testarray')));
}
