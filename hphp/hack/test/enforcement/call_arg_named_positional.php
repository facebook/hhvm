<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

function takes_named_positional<T>(T $value, named int $label): void {}

function test(): void {
  takes_named_positional(label = 1, 'value');
//                                   ^ enforcement-at-caret
}
