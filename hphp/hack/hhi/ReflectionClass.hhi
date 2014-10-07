<?hh // decl

class ReflectionClass {

  const int IS_IMPLICIT_ABSTRACT = 16;
  const int IS_EXPLICIT_ABSTRACT = 32;
  const int IS_FINAL = 64;

  /**
   * This field is read-only
   */
  public string $name;

  public function __construct(mixed $argument);
  public static function export(mixed $argument, bool $return = false): ?string;
  public function getConstant(string $name): mixed;
  public function getConstants(): array<string, mixed>;
  public function getConstructor(): ?ReflectionMethod;
  public function getDefaultProperties(): array<string, mixed>;
  /**
   * Returns string or false
   */
  public function getDocComment(): mixed;
  public function getEndLine(): int;
  public function getExtension(): ?ReflectionExtension;
  public function getExtensionName(): string;
  /**
   * Returns string or false
   */
  public function getFileName(): mixed;
  public function getInterfaceNames(): array<string>;
  public function getInterfaces(): array<string, ReflectionClass>;
  public function getMethod(string $name): ReflectionMethod;
  public function getMethods(?int $filter = null): array<ReflectionMethod>;
  public function getModifiers(): int;
  public function getName(): string;
  public function getNamespaceName(): string;
  /**
   * Returns ReflectionClass or false
   */
  public function getParentClass(): mixed;
  public function getProperties(int $filter = 0xFFFF): array<ReflectionProperty>;
  public function getProperty(string $name): ReflectionProperty;
  public function getRequirementNames(): array<string>;
  public function getRequirements(): array<string, ReflectionClass>;
  public function getShortName(): string;
  public function getStartLine(): int;
  public function getStaticProperties(): array<string, ReflectionProperty>;
  public function getStaticPropertyValue(string $name, mixed $def_value = null): mixed;
  public function getTraitAliases(): array<string, string>;
  public function getTraitNames(): array<string>;
  public function getTraits(): array<string, ReflectionClass>;
  public function hasConstant(string $name): bool;
  public function hasMethod(string $name): bool;
  public function hasProperty(string $name): bool;
  public function implementsInterface(string $interface): bool;
  public function inNamespace(): bool;
  public function isAbstract(): bool;
  public function isCloneable(): bool;
  public function isFinal(): bool;
  public function isInstance(mixed $object): bool;
  public function isInstantiable(): bool;
  public function isInterface(): bool;
  public function isInternal(): bool;
  public function isIterateable(): bool;
  /**
   * $class is string or ReflectionClass
   */
  public function isSubclassOf(mixed $class): bool;
  public function isTrait(): bool;
  public function isUserDefined(): bool;
  public function newInstance(...$args);
  public function newInstanceArgs(Traversable<mixed> $args = array());
  public function newInstanceWithoutConstructor();
  public function setStaticPropertyValue(string $name, string $value): void;
  public function __toString(): string;

}
