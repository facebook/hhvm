<?hh
// This tests the parser. The actual values are arbitrary.

class Bar {
  public static function foreach(): string {
    return "turtles";
  }
}
class Foo {
  public function callable(): int {
    return 0;
  }
  public function class(): int {
    return 1;
  }
  public function trait(): int {
    return 2;
  }
  public function extends(): int {
    return 3;
  }
  public function implements(): int {
    return 4;
  }
  public function static(): int {
    return 5;
  }
  public function abstract(): int {
    return 6;
  }
  public function final(): int {
    return 7;
  }
  public function public(): int {
    return 8;
  }
  public function protected(): int {
    return 9;
  }
  public function private(): int {
    return 10;
  }
  public function const(): int {
    return 11;
  }
  public function enddeclare(): int {
    return 12;
  }
  public function endfor(): int {
    return 13;
  }
  public function endforeach(): int {
    return 14;
  }
  public function endif(): int {
    return 15;
  }
  public function endwhile(): int {
    return 16;
  }
  public function and(): int {
    return 17;
  }
  public function global(): int {
    return 18;
  }
  public function goto(): int {
    return 19;
  }
  public function instanceof(): int {
    return 20;
  }
  public function insteadof(): int {
    return 21;
  }
  public function interface(): int {
    return 22;
  }
  public function namespace(): int {
    return 23;
  }
  public function new(): int {
    return 24;
  }
  public function or(): int {
    return 25;
  }
  public function xor(): int {
    return 26;
  }
  public function try(): int {
    return 27;
  }
  public function use(): int {
    return 28;
  }
  public function var(): int {
    return 29;
  }
  public function exit(): int {
    return 30;
  }
  public function list(): int {
    return 31;
  }
  public function clone(): int {
    return 32;
  }
  public function include(): int {
    return 33;
  }
  public function include_once(): int {
    return 34;
  }
  public function throw(): int {
    return 35;
  }
  public function array(): int {
    return 36;
  }
  public function print(): int {
    return 37;
  }
  public function echo(): int {
    return 38;
  }
  public function require(): int {
    return 39;
  }
  public function require_once(): int {
    return 40;
  }
  public function return(): int {
    return 41;
  }
  public function else(): int {
    return 42;
  }
  public function default(): int {
    return 44;
  }
  public function break(): int {
    return 45;
  }
  public function continue(): int {
    return 46;
  }
  public function switch(): int {
    return 47;
  }
  public function yield(): int {
    return 48;
  }
  public function function(): int {
    return 49;
  }
  public function if(): int {
    return 50;
  }
  public function endswitch(): int {
    return 51;
  }
  public function finally(): int {
    return 52;
  }
  public function for(): int {
    return 53;
  }
  public function foreach(): int {
    return 54;
  }
  public function declare(): int {
    return 55;
  }
  public function case(): int {
    return 56;
  }
  public function do(): int {
    return 57;
  }
  public function while(): int {
    return 58;
  }
  public function as(): int {
    return 59;
  }
  public function catch(): int {
    return 60;
  }
  // public function exit(): int {
  //   return 61;
  // }
  public function self(): int {
    return 62;
  }
  public function parent(): int {
    return 63;
  }
  public function unset(): int {
    return 64;
  }
}


<<__EntryPoint>>
function main_object_methods() :mixed{
echo Bar::foreach();
$foo = new Foo();
echo $foo->callable();
echo $foo->class();
echo $foo->trait();
echo $foo->extends();
echo $foo->implements();
echo $foo->static();
echo $foo->abstract();
echo $foo->final();
echo $foo->public();
echo $foo->protected();
echo $foo->private();
echo $foo->const();
echo $foo->enddeclare();
echo $foo->endfor();
echo $foo->endforeach();
echo $foo->endif();
echo $foo->endwhile();
echo $foo->and();
echo $foo->global();
echo $foo->goto();
echo $foo->instanceof();
echo $foo->insteadof();
echo $foo->interface();
echo $foo->namespace();
echo $foo->new();
echo $foo->or();
echo $foo->xor();
echo $foo->try();
echo $foo->use();
echo $foo->var();
echo $foo->exit();
echo $foo->list();
echo $foo->clone();
echo $foo->include();
echo $foo->include_once();
echo $foo->throw();
echo $foo->array();
echo $foo->print();
echo $foo->echo();
echo $foo->require();
echo $foo->require_once();
echo $foo->return();
echo $foo->else();
echo $foo->default();
echo $foo->break();
echo $foo->continue();
echo $foo->switch();
echo $foo->yield();
echo $foo->function();
echo $foo->if();
echo $foo->endswitch();
echo $foo->finally();
echo $foo->for();
echo $foo->foreach();
echo $foo->declare();
echo $foo->case();
echo $foo->do();
echo $foo->while();
echo $foo->as();
echo $foo->catch();
//echo $foo->exit();
echo $foo->self();
echo $foo->parent();
echo $foo->unset();
echo "Done\n";
}
