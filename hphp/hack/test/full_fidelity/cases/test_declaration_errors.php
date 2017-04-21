<?hh
interface I { }
class B { }
class D extends B
{
  require extends B; // error
  require implements I; // error
}
interface I2
{
  require extends B; // no error
  require implements I; // error
}
trait T
{
  require extends B; // no error
  require implements I; // no error
}
