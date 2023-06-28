<?hh

interface Example {
    public static function sillyError():mixed;
}

class ExampleImpl implements Example {
    public static function sillyError() :mixed{
        echo "I am a silly error\n";
    }
}
<<__EntryPoint>> function main(): void {
ExampleImpl::sillyError();
}
