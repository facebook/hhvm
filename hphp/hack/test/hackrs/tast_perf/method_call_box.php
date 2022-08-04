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
    $box->set(4);
    $y = $box->get();
}
