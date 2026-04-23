<?hh

// Test that expression-dependent types are cleaned up after await
// statements, preventing exponential blowup when multiple builder chains
// appear in the same function. This is the sequential-statement
// analogue of tpenv_leak_across_lambdas.php (which tests lambdas).

abstract class AwaitRepFieldSettings {
  abstract const type TValue;
}

interface IAwaitRepField<+TSettings as AwaitRepFieldSettings> {}

class AwaitRepIntSettings extends AwaitRepFieldSettings {
  const type TValue = int;
}

class AwaitRepIntField implements IAwaitRepField<AwaitRepIntSettings> {}

enum class AwaitRepAccessors: IAwaitRepField<AwaitRepFieldSettings> {
  AwaitRepIntField A = new AwaitRepIntField();
  AwaitRepIntField B = new AwaitRepIntField();
  AwaitRepIntField C = new AwaitRepIntField();
  AwaitRepIntField D = new AwaitRepIntField();
  AwaitRepIntField E = new AwaitRepIntField();
}

interface IAwaitRepBuilderMethods<+TChainableBuilder as IAwaitRepBuilder> {
  public function setField<TValue>(
    HH\EnumClass\Label<AwaitRepAccessors, IAwaitRepField<
      AwaitRepFieldSettings with { type TValue super TValue },
    >> $label,
    TValue $value,
  ): TChainableBuilder;

  public function genSave(): Awaitable<void>;
}

interface IAwaitRepBuilder
  extends IAwaitRepBuilderMethods<this::TChainableBuilder> {
  abstract const type TChainableBuilder as
    IAwaitRepBuilder with { type TChainableBuilder = this::TChainableBuilder };
}

function make_builder(): IAwaitRepBuilder {
  throw new Exception("test stub");
}

async function tpenv_leak_across_awaits_repro(): Awaitable<void> {
  // Each of these await chains creates expression-dependent types.
  // Without cleanup, the tpenv accumulates bounds across chains,
  // causing exponential blowup.
  await make_builder()
    ->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)
    ->setField(#D, 4)->setField(#E, 5)->genSave();
  await make_builder()
    ->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)
    ->setField(#D, 4)->setField(#E, 5)->genSave();
  await make_builder()
    ->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)
    ->setField(#D, 4)->setField(#E, 5)->genSave();
  await make_builder()
    ->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)
    ->setField(#D, 4)->setField(#E, 5)->genSave();
  await make_builder()
    ->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)
    ->setField(#D, 4)->setField(#E, 5)->genSave();
  await make_builder()
    ->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)
    ->setField(#D, 4)->setField(#E, 5)->genSave();
  await make_builder()
    ->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)
    ->setField(#D, 4)->setField(#E, 5)->genSave();
  await make_builder()
    ->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)
    ->setField(#D, 4)->setField(#E, 5)->genSave();
  await make_builder()
    ->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)
    ->setField(#D, 4)->setField(#E, 5)->genSave();
  await make_builder()
    ->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)
    ->setField(#D, 4)->setField(#E, 5)->genSave();
}
