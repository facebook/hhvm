<?hh

<<__SupportDynamicType>>
function f(): void {}

function h(): void {
    $x = false && (f<> upcast dynamic is nonnull);
}
