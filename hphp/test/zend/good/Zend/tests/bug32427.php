<?hh

interface Example {
    public static function sillyError();
}

class ExampleImpl implements Example {
    public static function sillyError() {
        echo "I am a silly error\n";
    }
}
<<__EntryPoint>> function main(): void {
ExampleImpl::sillyError();
}
