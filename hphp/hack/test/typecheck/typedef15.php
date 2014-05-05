<?hh

type Metadata = Map<int, int>;
type HandlerTable = Map<int, Metadata>;

class Router {
  protected static Vector<Pair<HandlerTable, int>> $routes = Vector {};

  public function f(): Pair<HandlerTable, int> {
    $r = self::$routes[1];
    return $r;
  }
}
