<?hh

interface IGenericClass<+T> {}

function my_takes(IGenericClass<int> $_): void {}

case type CT = Container<int> | IGenericClass<int>;

function my_fun(CT $val): void {
  if ($val is IGenericClass<_>) {
    my_takes($val);
  }
}
