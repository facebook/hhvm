<?hh
class C implements Compare {
    public function eq(C $c_obj): bool {
        $c_obj->content();
        return True;
    }

    public function content(): void {

    }
}

interface Compare {
    public function eq(C $c_obj): bool;
}
