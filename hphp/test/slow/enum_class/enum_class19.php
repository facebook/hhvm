<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

interface IEnumVal {
    public function get(): mixed;
}

final class EnumVal implements IEnumVal  {
    private function __construct(private mixed $val): void {
    }

    public function get(): mixed {
        return $this->val;
    }

    public static function ok(): this {
        return new self('ok');
    }

    public static function bad(): this {
        invariant(false, 'Fail'); //Will produce a `\HH\InvariantException`.
        return new self('bad');
    }
}

enum class E: IEnumVal {
    EnumVal ok = EnumVal::ok();
    EnumVal bad = EnumVal::bad();
}

abstract final class UseE {
    public static function ok(): HH\MemberOf<E, IEnumVal> {
        return E::ok;
    }
    public static function bad(): HH\MemberOf<E, IEnumVal> {
        return E::bad;
    }
}

<<__EntryPoint>>
function main(): void {
    try {
        $ok = UseE::ok();
        $bad = UseE::bad();
    } catch (Exception $e) {
        echo 'Exception: '.$e->getMessage()."\n";
    }
}
