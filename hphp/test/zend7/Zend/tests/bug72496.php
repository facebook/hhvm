<?php
class Foo
{
    private function getFoo()
    {
        return 'Foo';
    }

    private function getBar()
    {
        return 'Bar';
    }

    private function getBaz()
    {
        return 'Baz';
    }
}

class Bar extends Foo
{
    public function getFoo($extraArgument)
    {
        return $extraArgument;
    }

    protected function getBar($extraArgument)
    {
        return $extraArgument;
    }

    private function getBaz($extraArgument)
    {
        return $extraArgument;
    }
}

echo "OK\n";
