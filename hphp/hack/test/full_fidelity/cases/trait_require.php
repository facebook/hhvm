<?hh // strict

trait T {
  require extends A;
}

trait U {
  require implements A;
}
