<?hh

class Demo {
    public function __construct(private readonly string $field) {}
        public readonly function field(): readonly string {
              return readonly $this->field;
        }
      public function exposeBug(): readonly string {
            return readonly $this->field();  // <-- error here
        }
}

