<?hh
/**


		My
Doc 
		* Comment 
for A

* */
class A {}

/** My DocComment for B */
class B extends A { }

class C extends B {}

/**
 * Interface doc comment
 */




interface I {}

/*.*
 * Not a doc comment
 */
class D implements I {}

/**** Not a doc comment */
class E extends C implements I {} 
/**?** Not a doc comment */
class F extends C implements I {} 
/**	** Doc comment for G */
final class G extends C implements I {} 

<<__EntryPoint>>
function main_entry(): void {

  $classes = vec['A', 'B', 'C', 'D', 'E', 'F', 'G', 'I'];
  foreach ($classes as $class) {
  	echo "\n\n---> Doc comment for class $class:\n";
  	$rc = new ReflectionClass($class);	
  	var_dump($rc->getDocComment());	
  }
}
