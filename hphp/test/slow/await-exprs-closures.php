<?hh

<<file:__EnableUnstableFeatures('allow_extended_await_syntax')>>

async function fc(
  int $idx,
  int $line,
  string $class,
  string $trait,
  string $method,
  string $func,
  string $static_name,
  string $self_name,
  string $parent_name,
  mixed $static_ptr,
  mixed $self_ptr,
  mixed $parent_ptr,
  FunctionCredential $fc
): Awaitable<int> {
  echo "[ $idx ] ==================================================\n";
  var_dump($class, $trait, $method, $func);
  var_dump($line);
  var_dump($static_name, $self_name, $parent_name);
  var_dump($static_ptr, $self_ptr, $parent_ptr);
  var_dump($fc->getClassName(), $fc->getFunctionName());
  var_dump(implode(", ", array_map($f ==> $f['function'], debug_backtrace())));
  return 0;
}

async function ff(
  int $idx,
  int $line,
  string $func,
  FunctionCredential $fc
): Awaitable<int> {
  echo "[ $idx ] ==================================================\n";
  var_dump($func);
  var_dump($line);
  var_dump($fc->getFunctionName());
  var_dump(implode(", ", array_map($f ==> $f['function'], debug_backtrace())));
  echo "\n";
  return 0;
}

async function g(...$vals) {
  return 0;
}

class P {}

trait T {
  public async function t() {
    $i = 36;
    await g(
      await g(
        await fc($i + 0, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
        await fc($i + 1, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
        await fc($i + 2, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
        await g(
          await fc($i + 3, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__),
          await fc($i + 4, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__),
          await fc($i + 5, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__),
        ) +
        await g(
          await fc($i + 6, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__),
          await fc($i + 7, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__),
          await fc($i + 8, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__),
        ),
        await fc($i + 9, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
        await fc($i + 10, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
      ),
      await g(
        await fc($i + 11, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
        await fc($i + 12, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
      ) + 
      await g(
        await fc($i + 13, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
        await fc($i + 14, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
      ) +
      await g(
        await async {
          await fc($i + 15, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__);
          return await (() ==>
            fc($i + 16, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                     __FUNCTION__, nameof static, nameof self, nameof parent,
                     static::class, self::class, parent::class,
                     __FUNCTION_CREDENTIAL__)
          )();
        } +
        await fc($i + 17, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__)
      )
    );
  }
}

class C extends P {
  use T;

  public async function m() {
    $i = 18;
    await g(
      await g(
        await fc($i + 0, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
        await fc($i + 1, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
        await fc($i + 2, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
        await g(
          await fc($i + 3, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__),
          await fc($i + 4, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__),
          await fc($i + 5, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__),
        ) +
        await g(
          await fc($i + 6, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__),
          await fc($i + 7, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__),
          await fc($i + 8, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__),
        ),
        await fc($i + 9, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
        await fc($i + 10, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
      ),
      await g(
        await fc($i + 11, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
        await fc($i + 12, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
      ) + 
      await g(
        await fc($i + 13, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
        await fc($i + 14, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__),
      ) +
      await g(
        await async {
          await fc($i + 15, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                   __FUNCTION__, nameof static, nameof self, nameof parent,
                   static::class, self::class, parent::class,
                   __FUNCTION_CREDENTIAL__);
          return await (() ==>
            fc($i + 16, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                     __FUNCTION__, nameof static, nameof self, nameof parent,
                     static::class, self::class, parent::class,
                     __FUNCTION_CREDENTIAL__)
          )();
        } +
        await fc($i + 17, __LINE__, __CLASS__, __TRAIT__, __METHOD__,
                 __FUNCTION__, nameof static, nameof self, nameof parent,
                 static::class, self::class, parent::class,
                 __FUNCTION_CREDENTIAL__)
      )
    );
  }
}

class X extends C {}

<<__EntryPoint>>
async function main() {
  await g(
    await g(
      await ff(0, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
      await ff(1, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
      await ff(2, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
      await g(
        await ff(3, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
        await ff(4, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
        await ff(5, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
      ) +
      await g(
        await ff(6, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
        await ff(7, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
        await ff(8, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
      ),
      await ff(9, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
      await ff(10, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
    ),
    await g(
      await ff(11, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
      await ff(12, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
    ) + 
    await g(
      await ff(13, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
      await ff(14, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
    ) +
    await g(
      await async {
        await ff(15, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__);
        return await (() ==> ff(16, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__))();
      } +
      await ff(17, __LINE__, __FUNCTION__, __FUNCTION_CREDENTIAL__),
    )
  );


  echo "===========================================================\n";

  new X()->m();

  echo "===========================================================\n";

  new X()->t();
}
