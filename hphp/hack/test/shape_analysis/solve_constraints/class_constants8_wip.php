<?hh

class C1 {
    const dict<string, mixed> DICT = dict['a' => 2];
    // type incorrectly solved to: shape('a' => int, ?'extra' => mixed)
    // better to not convert to dict,
}


function fn(): void {
    C1::DICT['extra']; // OutOfBoundsException
}
