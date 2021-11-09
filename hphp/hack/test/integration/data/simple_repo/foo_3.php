<?hh

        function h(): string {
            return "a";
        }

        class Foo {}

        function some_long_function_name(): void {
            new Foo();
            h();
        }
