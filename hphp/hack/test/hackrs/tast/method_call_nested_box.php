<?hh

interface IBox<T> {
    public function set(T $x): void;
    public function get(): T;
}

interface IBoxFactory {
    public function new_box<T>(): IBox<T>;
}

function foo(IBoxFactory $f): void {
    $box = $f->new_box();
    $nested_box = $f->new_box();
    $nested_box->set(4);
    $box->set($nested_box);
    $y = $box->get()->get();
}
