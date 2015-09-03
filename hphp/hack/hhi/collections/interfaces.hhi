<?hh // decl

/**
 * The base interface implemented for a collection type so that base information
 * such as count and its items are available.
 *
 */
interface ConstCollection<+Te> extends Countable {
  /**
   * Is the collection empty?
   *
   * @return bool - Returns TRUE if the collection is empty; FASLE otherswise.
   */
  public function isEmpty(): bool;
  /**
   * Get the number of items in the collection. Cannot be negative.
   *
   * @return int - Returns the number of items in the collection
   */
  public function count(): int;
  /**
   * Get access to the items in the collection. Can be empty.
   *
   * @return Iterable - Returns an iterable with access to all of the items in
   *   the collection.
   */
  public function items(): Iterable<Te>;
}

interface OutputCollection<-Te> {
  public function add(Te $e): this;
  public function addAll(?Traversable<Te> $traversable): this;
}

/**
 * Collection is the primary collection interface for mutable collections.
 * Assuming you want the ability to clear out your collection, you would
 * implement this (or a child of this) interface. Otherwise, you can implement
 * @OutputCollection only. If your collection to be immutable, implement
 * @ConstCollection only instead.
 */
interface Collection<Te> extends ConstCollection<Te>,
                                 OutputCollection<Te> {
  /**
   * Removes all items from the collection
   *
   * @return void
   */
  public function clear();
}

interface ConstSetAccess<+Tm> {
  public function contains<Tu super Tm>(Tu $m): bool;
}

interface SetAccess<Tm> extends ConstSetAccess<Tm> {
  public function remove(Tm $m): this;
}

interface ConstIndexAccess<Tk, +Tv> {
  public function at(Tk $k): Tv;
  public function get(Tk $k): ?Tv;
  public function containsKey<Tu super Tk>(Tu $k): bool;
}

interface IndexAccess<Tk, Tv> extends ConstIndexAccess<Tk, Tv> {
  public function set(Tk $k, Tv $v): this;
  public function setAll(?KeyedTraversable<Tk, Tv> $traversable): this;
  public function removeKey(Tk $k): this;
}

interface ConstMapAccess<Tk, +Tv> extends ConstSetAccess<Tk>,
                                          ConstIndexAccess<Tk, Tv> {
}

interface MapAccess<Tk, Tv> extends ConstMapAccess<Tk, Tv>,
                                    SetAccess<Tk>,
                                    IndexAccess<Tk, Tv> {
}

interface ConstVector<+Tv> extends ConstCollection<Tv>,
                                   ConstIndexAccess<int, Tv>,
                                   KeyedIterable<int, Tv>,
                                   Indexish<int, Tv> {
  public function values(): ConstVector<Tv>;
  public function keys(): ConstVector<int>;
  public function map<Tu>((function(Tv): Tu) $fn): ConstVector<Tu>;
  public function mapWithKey<Tu>((function(int, Tv): Tu) $fn):
    ConstVector<Tu>;
  public function filter((function(Tv): bool) $fn): ConstVector<Tv>;
  public function filterWithKey((function(int, Tv): bool) $fn):
    ConstVector<Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable): ConstVector<Pair<Tv, Tu>>;
  public function take(int $n): ConstVector<Tv>;
  public function takeWhile((function(Tv): bool) $fn): ConstVector<Tv>;
  public function skip(int $n): ConstVector<Tv>;
  public function skipWhile((function(Tv): bool) $fn): ConstVector<Tv>;
  public function slice(int $start, int $len): ConstVector<Tv>;
  public function concat<Tu super Tv>(Traversable<Tu> $traversable): ConstVector<Tu>;
  public function firstValue(): ?Tv;
  public function firstKey(): ?int;
  public function lastValue(): ?Tv;
  public function lastKey(): ?int;
  public function linearSearch<Tu super Tv>(Tu $search_value): int;
}

interface MutableVector<Tv> extends ConstVector<Tv>,
                                    Collection<Tv>,
                                    IndexAccess<int, Tv> {
  public function values(): MutableVector<Tv>;
  public function keys(): MutableVector<int>;
  public function map<Tu>((function(Tv): Tu) $fn): MutableVector<Tu>;
  public function mapWithKey<Tu>((function(int, Tv): Tu) $fn):
    MutableVector<Tu>;
  public function filter((function(Tv): bool) $fn): MutableVector<Tv>;
  public function filterWithKey((function(int, Tv): bool) $fn):
    MutableVector<Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable):
    MutableVector<Pair<Tv, Tu>>;
  public function take(int $n): MutableVector<Tv>;
  public function takeWhile((function(Tv): bool) $fn): MutableVector<Tv>;
  public function skip(int $n): MutableVector<Tv>;
  public function skipWhile((function(Tv): bool) $fn): MutableVector<Tv>;
  public function slice(int $start, int $len): MutableVector<Tv>;
  public function concat<Tu super Tv>(Traversable<Tu> $traversable): MutableVector<Tu>;
  public function firstValue(): ?Tv;
  public function firstKey(): ?int;
  public function lastValue(): ?Tv;
  public function lastKey(): ?int;
  public function linearSearch<Tu super Tv>(Tu $search_value): int;
}

interface ConstMap<Tk, +Tv> extends ConstCollection<Pair<Tk, Tv>>,
                                    ConstMapAccess<Tk, Tv>,
                                    KeyedIterable<Tk, Tv>,
                                    Indexish<Tk, Tv> {
  public function values(): ConstVector<Tv>;
  public function keys(): ConstVector<Tk>;
  public function map<Tu>((function(Tv): Tu) $fn): ConstMap<Tk, Tu>;
  public function mapWithKey<Tu>((function(Tk, Tv): Tu) $fn):
    ConstMap<Tk, Tu>;
  public function filter((function(Tv): bool) $fn): ConstMap<Tk, Tv>;
  public function filterWithKey((function(Tk, Tv): bool) $fn):
    ConstMap<Tk, Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable):
    ConstMap<Tk, Pair<Tv, Tu>>;
  public function take(int $n): ConstMap<Tk, Tv>;
  public function takeWhile((function(Tv): bool) $fn): ConstMap<Tk, Tv>;
  public function skip(int $n): ConstMap<Tk, Tv>;
  public function skipWhile((function(Tv): bool) $fn): ConstMap<Tk, Tv>;
  public function slice(int $start, int $len): ConstMap<Tk, Tv>;
  public function concat<Tu super Tv>(Traversable<Tu> $traversable): ConstVector<Tu>;
  public function firstValue(): ?Tv;
  public function firstKey(): ?Tk;
  public function lastValue(): ?Tv;
  public function lastKey(): ?Tk;
}

interface MutableMap<Tk, Tv> extends ConstMap<Tk, Tv>,
                                     Collection<Pair<Tk, Tv>>,
                                     MapAccess<Tk, Tv> {
  public function values(): MutableVector<Tv>;
  public function keys(): MutableVector<Tk>;
  public function map<Tu>((function(Tv): Tu) $fn): MutableMap<Tk, Tu>;
  public function mapWithKey<Tu>((function(Tk, Tv): Tu) $fn):
    MutableMap<Tk, Tu>;
  public function filter((function(Tv): bool) $fn): MutableMap<Tk, Tv>;
  public function filterWithKey((function(Tk, Tv): bool) $fn):
    MutableMap<Tk, Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable):
    MutableMap<Tk, Pair<Tv, Tu>>;
  public function take(int $n): MutableMap<Tk, Tv>;
  public function takeWhile((function(Tv): bool) $fn): MutableMap<Tk, Tv>;
  public function skip(int $n): MutableMap<Tk, Tv>;
  public function skipWhile((function(Tv): bool) $fn): MutableMap<Tk, Tv>;
  public function slice(int $start, int $len): MutableMap<Tk, Tv>;
  public function concat<Tu super Tv>(Traversable<Tu> $traversable): MutableVector<Tu>;
  public function firstValue(): ?Tv;
  public function firstKey(): ?Tk;
  public function lastValue(): ?Tv;
  public function lastKey(): ?Tk;
}

interface ConstSet<+Tv> extends ConstCollection<Tv>,
                                ConstSetAccess<Tv>,
                                KeyedIterable<mixed, Tv>,
                                Container<Tv> {
  public function values(): ConstVector<Tv>;
  public function keys(): ConstVector<mixed>;
  public function map<Tu>((function(Tv): Tu) $fn): ConstSet<Tu>;
  public function mapWithKey<Tu>((function(mixed, Tv): Tu) $fn): ConstSet<Tu>;
  public function filter((function(Tv): bool) $fn): ConstSet<Tv>;
  public function filterWithKey((function(mixed, Tv): bool) $fn): ConstSet<Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable): ConstSet<Pair<Tv, Tu>>;
  public function take(int $n): ConstSet<Tv>;
  public function takeWhile((function(Tv): bool) $fn): ConstSet<Tv>;
  public function skip(int $n): ConstSet<Tv>;
  public function skipWhile((function(Tv): bool) $fn): ConstSet<Tv>;
  public function slice(int $start, int $len): ConstSet<Tv>;
  public function concat<Tu super Tv>(Traversable<Tu> $traversable): ConstVector<Tu>;
  public function firstValue(): ?Tv;
  public function firstKey(): mixed;
  public function lastValue(): ?Tv;
  public function lastKey(): mixed;
}

interface MutableSet<Tv> extends ConstSet<Tv>,
                                 Collection<Tv>,
                                 SetAccess<Tv> {
  public function values(): MutableVector<Tv>;
  public function keys(): MutableVector<mixed>;
  public function map<Tu>((function(Tv): Tu) $fn): MutableSet<Tu>;
  public function mapWithKey<Tu>((function(mixed, Tv): Tu) $fn): MutableSet<Tu>;
  public function filter((function(Tv): bool) $fn): MutableSet<Tv>;
  public function filterWithKey((function(mixed, Tv): bool) $fn): MutableSet<Tv>;
  public function zip<Tu>(Traversable<Tu> $traversable): MutableSet<Pair<Tv, Tu>>;
  public function take(int $n): MutableSet<Tv>;
  public function takeWhile((function(Tv): bool) $fn): MutableSet<Tv>;
  public function skip(int $n): MutableSet<Tv>;
  public function skipWhile((function(Tv): bool) $fn): MutableSet<Tv>;
  public function slice(int $start, int $len): MutableSet<Tv>;
  public function concat<Tu super Tv>(Traversable<Tu> $traversable): MutableVector<Tu>;
  public function firstValue(): ?Tv;
  public function firstKey(): mixed;
  public function lastValue(): ?Tv;
  public function lastKey(): mixed;
}
