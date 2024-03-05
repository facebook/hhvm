# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# pyre-unsafe

"""
Classes for generating random values for thrift types
"""

from __future__ import absolute_import, division, print_function, unicode_literals

import collections
import random
import sys

import six

# pyre-fixme[21]: Could not find module `six.moves`.
import six.moves as sm
from thrift import Thrift
from thrift.util.type_inspect import get_spec, ThriftPyTypeSpec


INFINITY = float("inf")

if sys.version_info[0] >= 3:
    unicode = None


def deep_dict_update(base, update) -> None:
    """Similar to dict.update(base, update), but if any values in base are
    dictionaries, they are updated too instead of replaced.

    Destructive on base, but non-destructive on base's values.
    """
    for key, val in six.iteritems(update):
        if key in base and isinstance(base[key], dict) and isinstance(val, dict):
            # Copy base[key] (non-destructive on base's values)
            updated = dict(base[key])
            deep_dict_update(updated, val)
            val = updated
        base[key] = val


class BaseRandomizer(object):
    """
    The BaseRandomizer class is an abstract class whose subclasses implement
    a randomizer for a specific Thrift Type. Instances of a class may have
    different spec_args and constraints.

    Class Attributes:

    ttype (int (enum)): The attribute of Thrift.TTypes corresponding to the type

    default_constraints (dict): Default values for randomizers' constraint
    dictionary. Constraints affect the behavior of the randomize() method.

    Instance Attributes:

    type_spec (ThriftTypeSpec): The thrift spec wrapper. Provides additional
    information about the type beyond thrift type.

    state (RandomizerState): State attributes to be preserved across randomizer
    components in recursive and nested randomizer structures. Includes
    initialization cache and recursion depth trace.

    constraints (dict): Map of constraint names to constraint values. This
    is equivalent to cls.default_constraints if an empty constraint dictionary
    is passed to __init__. Otherwise, it is equal to cls.default_constraints
    recursively updated with the key/value pairs in the constraint dict passed
    to __init__.
    """

    ttype = None

    default_constraints = {
        "seeds": [],
        "p_random": 0.08,  # If seeded, chance of ignoring seed
        "p_fuzz": 1,  # If seed not ignored, chance of fuzzing seed
    }

    def __init__(self, type_spec, state, constraints):
        """
        spec_args: thrift arguments for this field
        state: RandomizerState instance
        constraints: dict of constraints specific to this randomizer
        """
        self.type_spec = type_spec
        self.state = state
        self.type_name = type_spec.get_type_name()
        self.constraints = self.flatten_constraints(constraints)
        self.preprocessing_done = False

    def _preprocess_constraints(self):
        pass

    def _init_subrandomizers(self):
        pass

    def preprocess(self):
        if self.preprocessing_done:
            return

        # Push type rules that may affect subrandomizers' constraints
        pushed = self.state.push_type_constraints(self.constraints)

        self._preprocess_constraints()
        self._init_subrandomizers()

        self.state.pop_type_constraints(pushed)

        self.preprocessing_done = True

    def flatten_constraints(self, constraints):
        """Return a single constraint dictionary by combining default
        constraints with overriding constraints."""
        cls = self.__class__

        flattened = {}
        deep_dict_update(flattened, cls.default_constraints)

        # Put the default constraints of the whole stack
        deep_dict_update(flattened, self.state.default_constraints)

        type_name = self.type_name
        for type_constraints in self.state.type_constraint_stacks[type_name]:
            deep_dict_update(flattened, type_constraints)

        deep_dict_update(flattened, constraints)

        return flattened

    def __eq__(self, other):
        """Check if this randomizer is equal to `other` randomizer. If two
        randomizers are equal, they have the same type and constraints and
        are expected to behave identically (up to random number generation.)"""
        return (self.type_spec == other.type_spec) and (
            self.constraints == other.constraints
        )

    @property
    def universe_size(self):
        """
        Return (or estimate) the range of the random variable. If this
        randomizer is used for sets or map keys, the size of the container
        will be limited to this value.
        """
        raise NotImplementedError(
            "_universe_size not implemented for %s" % (self.__class__.__name__)
        )

    def generate(self, seed=None):
        """Generate a value, possibly based on a seed.

        If seed is not None, use it as the seed. Otherwise, if the seeds
        constraint is non-empty, pick a random element as the seed.

        If there are no seeds, return the result of randomize()

        If there are seeds, use the p_random constraint to determine the
        chance of returning the result of randomize() and use the p_fuzz
        constraint to determine the chance of returning the result of fuzz(seed)
        Otherwise, return the seed.
        """
        if seed is None:
            seeds = self.constraints["seeds"]
        else:
            seeds = [seed]

        if not seeds or (random.random() < self.constraints["p_random"]):
            return self._randomize()

        seed = random.choice(seeds)

        if random.random() < self.constraints["p_fuzz"]:
            return self._fuzz(seed)
        else:
            return self.eval_seed(seed)

    def _randomize(self):
        """Generate a random value of the type, given the spec args"""
        raise NotImplementedError(
            "randomize not implemented for %s" % (self.__class__.__name__)
        )

    def _fuzz(self, seed):
        """Randomly modify the given seed value.

        By default, this method calls _randomize() and returns a completely
        randomized value.

        However, subclasses for types whose values can be "close" to each
        other should override this method to randomly generate a value
        that is "close" to the seed. For example, an int randomizer might
        fuzz the seed 1000 by returning 1001. A string randomizer might fuzz
        the seed "foo" to "fOo".
        """
        return self._randomize()

    def eval_seed(self, seed):
        """Evaluate a seed without fuzzing it.

        Seeds must be specified as JSON, so they may not always match
        the type that this randomizer is expected to generate. This method
        converts the result of json.loads(seed) to a value with the expected
        thrift type.

        For example,
        an int seed may be "3", which evaluates to 3. A Point struct seed may
        be {"x": 4, "y": 2}, which evaluates to Point(x=4, y=2).
        """
        return seed


class ScalarTypeRandomizer(BaseRandomizer):
    """Randomizer for types that do not constain other types, including
    enum, byte, i16, i32, i64, float, double and string. Bool is excluded
    because it does not need to inherit any properties from this class"""

    default_constraints = dict(BaseRandomizer.default_constraints)
    default_constraints.update({"choices": []})

    def _randomize(self):
        """Basic types support the choices constraint, which restricts
        the range of the randomizer to an explicit list"""
        choices = self.constraints["choices"]
        if choices:
            return random.choice(choices)
        else:
            return None


class BoolRandomizer(BaseRandomizer):
    ttype = Thrift.TType.BOOL

    default_constraints = dict(BaseRandomizer.default_constraints)
    default_constraints.update({"p_true": 0.5})

    @property
    def universe_size(self):
        return 2

    def _randomize(self):
        return random.random() < self.constraints["p_true"]

    def eval_seed(self, seed):
        if isinstance(seed, bool):
            return seed
        elif isinstance(seed, six.integer_types):
            return bool(seed)
        elif seed == "true":
            return True
        elif seed == "false":
            return False
        else:
            raise ValueError("Invalid bool seed: %s" % seed)


def _random_int_factory(k):
    """Return a function that generates a random k-bit signed int"""
    min_ = -(1 << (k - 1))  # -2**(k-1)
    max_ = 1 << (k - 1) - 1  # +2**(k-1)-1

    def random_int_k_bits():
        return random.randint(min_, max_)

    return random_int_k_bits


class EnumRandomizer(ScalarTypeRandomizer):
    ttype = Thrift.TType.I32

    random_int_32 = staticmethod(_random_int_factory(32))

    default_constraints = dict(ScalarTypeRandomizer.default_constraints)
    default_constraints.update(
        {
            # Probability of generating an i32 with no corresponding Enum name
            "p_invalid": 0.01,
        }
    )

    def _preprocess_constraints(self):
        self._whiteset = set()
        for val in six.itervalues(self.type_spec.names_to_values()):
            self._whiteset.add(val)

        self._whitelist = list(self._whiteset)

    def _randomize(self):
        cls = self.__class__

        val = super(EnumRandomizer, self)._randomize()
        if val is not None:
            if isinstance(val, six.string_types):
                return self.type_spec.names_to_values()[val]
            else:
                return self.type_spec.construct_instance(val)

        if random.random() < self.constraints["p_invalid"]:
            # Generate an i32 value that does not correspond to an enum member
            n = None
            while (n in self._whiteset) or (n is None):
                n = cls.random_int_32()
            return self.type_spec.construct_instance(n)
        else:
            return self.type_spec.construct_instance(random.choice(self._whitelist))

    def eval_seed(self, seed):
        if isinstance(seed, six.string_types):
            seed = self.type_spec.names_to_values()[seed]
        elif not isinstance(seed, int):
            # Assume the seed is given in its native type
            return seed
        return self.type_spec.construct_instance(seed)

    @property
    def universe_size(self):
        return len(self._whitelist)


def _integer_randomizer_factory(name, ttype, n_bits):
    _universe_size = 2**n_bits
    _min = -(2 ** (n_bits - 1))
    _max = (2 ** (n_bits - 1)) - 1
    _name = name
    _ttype = ttype
    _n_bits = n_bits
    _random_i32 = _random_int_factory(_n_bits)

    class NBitIntegerRandomizer(ScalarTypeRandomizer):
        ttype = _ttype

        default_constraints = dict(ScalarTypeRandomizer.default_constraints)
        default_constraints.update({"range": [], "fuzz_max_delta": 4})

        def _randomize(self):
            val = super(NBitIntegerRandomizer, self)._randomize()

            if val is not None:
                return val

            range_ = self.constraints["range"]
            if range_:
                min_, max_ = range_
                return random.randint(min_, max_)

            return _random_i32()

        def _flip_bit(self, seed):
            """Fuzz seed by flipping one bit, excluding the sign bit"""
            flipper = 1 << random.randint(0, _n_bits - 2)
            return seed ^ flipper

        def _add_delta(self, seed):
            """Fuzz seed by adding a small number"""
            max_delta = self.constraints["fuzz_max_delta"]
            delta = random.randint(-max_delta, max_delta)
            fuzzed = seed + delta

            # Make sure fuzzed is in [_min, _max] to avoid overflow
            return max(min(_max, fuzzed), _min)

        def _fuzz(self, seed):
            """Apply a random fuzzer function"""
            seed = self.eval_seed(seed)
            fuzz_fn = random.choice([self._flip_bit, self._add_delta])
            return fuzz_fn(seed)

        @property
        def universe_size(self):
            return _universe_size

        def eval_seed(self, seed):
            if isinstance(seed, six.string_types):
                return int(seed)
            elif isinstance(seed, six.integer_types):
                return seed
            else:
                raise TypeError("Invalid %s seed: %s" % (_name, seed))

    if sys.version_info[0] == 2 and isinstance(_name, unicode):
        NBitIntegerRandomizer.__name__ = "{}Randomizer".format(_name).encode("utf8")
    else:
        NBitIntegerRandomizer.__name__ = "{}Randomizer".format(_name)

    return NBitIntegerRandomizer


ByteRandomizer = _integer_randomizer_factory("i8", Thrift.TType.BYTE, 8)
I16Randomizer = _integer_randomizer_factory("i16", Thrift.TType.I16, 16)
I32Randomizer = _integer_randomizer_factory("i32", Thrift.TType.I32, 32)
I64Randomizer = _integer_randomizer_factory("i64", Thrift.TType.I64, 64)

del _integer_randomizer_factory


class FloatingPointRandomizer(ScalarTypeRandomizer):
    """Abstract class for floating point types"""

    unreals = [float("nan"), float("inf"), float("-inf")]

    default_constraints = dict(ScalarTypeRandomizer.default_constraints)
    default_constraints.update(
        {
            "p_zero": 0.01,
            "p_unreal": 0.01,
            "mean": 0.0,
            "std_deviation": 1e8,
        }
    )

    @property
    def universe_size(self):
        return self.__class__._universe_size

    def _randomize(self):
        cls = self.__class__

        val = super(FloatingPointRandomizer, self)._randomize()
        if val is not None:
            return val

        if random.random() < self.constraints["p_unreal"]:
            return random.choice(cls.unreals)

        if random.random() < self.constraints["p_zero"]:
            return 0.0

        return random.normalvariate(
            self.constraints["mean"], self.constraints["std_deviation"]
        )

    def eval_seed(self, seed):
        if isinstance(seed, six.string_types):
            return float(seed)
        elif isinstance(seed, (float,) + six.integer_types):
            return seed
        else:
            raise TypeError("Invalid %s seed: %s" % (self.__class__.name, seed))


class SinglePrecisionFloatRandomizer(FloatingPointRandomizer):
    ttype = Thrift.TType.FLOAT

    _universe_size = 2**32


class DoublePrecisionFloatRandomizer(FloatingPointRandomizer):
    ttype = Thrift.TType.DOUBLE

    _universe_size = 2**64


class CollectionTypeRandomizer(BaseRandomizer):
    """Superclass for ttypes with lengths"""

    default_constraints = dict(BaseRandomizer.default_constraints)
    default_constraints.update({"mean_length": 12})

    @property
    def universe_size(self):
        return INFINITY

    def _get_length(self):
        mean = self.constraints["mean_length"]
        if mean == 0:
            return 0
        else:
            val = int(random.expovariate(1 / mean))
            max_len = self.constraints.get("max_length", None)
            if max_len is not None and val > max_len:
                val = max_len
            return val


class StringRandomizer(CollectionTypeRandomizer, ScalarTypeRandomizer):
    ttype = Thrift.TType.STRING

    ascii_range = (0, 127)

    default_constraints = dict(CollectionTypeRandomizer.default_constraints)
    default_constraints.update(ScalarTypeRandomizer.default_constraints)

    def _randomize(self):
        cls = self.__class__

        val = ScalarTypeRandomizer._randomize(self)
        if val is not None:
            return val

        length = self._get_length()
        chars = []

        for _ in sm.xrange(length):
            chars.append(chr(random.randint(*cls.ascii_range)))

        return "".join(chars)

    def eval_seed(self, seed):
        if isinstance(seed, six.string_types):
            return seed
        elif isinstance(seed, six.binary_type):
            return seed
        else:
            raise TypeError("Invalid string seed: %s" % seed)


class BinaryRandomizer(CollectionTypeRandomizer, ScalarTypeRandomizer):
    ttype = Thrift.TType.UTF8

    byte_range = (0, 255)

    default_constraints = dict(CollectionTypeRandomizer.default_constraints)
    default_constraints.update(ScalarTypeRandomizer.default_constraints)

    def _randomize(self):
        val = ScalarTypeRandomizer._randomize(self)
        if val is not None:
            return self.type_spec.construct_instance(val)

        length = self._get_length()
        bs = []

        for _ in sm.xrange(length):
            bs.append(six.int2byte(random.randint(*self.byte_range)))

        return self.type_spec.construct_instance(six.ensure_binary("").join(bs))

    def eval_seed(self, seed):
        if isinstance(seed, six.string_types):
            return self.type_spec.construct_instance(six.ensure_binary(seed))
        elif isinstance(seed, six.binary_type):
            return self.type_spec.construct_instance(seed)
        else:
            raise TypeError("Invalid binary seed: %s" % seed)


class NonAssociativeContainerRandomizer(CollectionTypeRandomizer):
    """Randomizer class for lists and sets"""

    default_constraints = dict(CollectionTypeRandomizer.default_constraints)
    default_constraints.update({"element": {}})

    def _init_subrandomizers(self):
        elem_spec = self.type_spec.get_subtypes()[ThriftPyTypeSpec.SUBTYPE_ELEMENT]
        elem_constraints = self.constraints["element"]
        self._element_randomizer = self.state.get_randomizer_for_spec(
            elem_spec, elem_constraints
        )


class ListRandomizer(NonAssociativeContainerRandomizer):
    ttype = Thrift.TType.LIST

    def _randomize(self):
        length = self._get_length()
        elements = []

        for _ in sm.xrange(length):
            element = self._element_randomizer.generate()
            if element is not None:
                elements.append(element)

        return self.type_spec.construct_instance(elements)

    def _fuzz_insert(self, seed):
        """Fuzz a list seed by inserting a random element at a random index"""
        randomizer = self._element_randomizer
        new_elem = randomizer.generate()
        insertion_index = random.randint(0, len(seed))
        seed.insert(insertion_index, new_elem)
        return seed

    def _fuzz_delete(self, seed):
        """Fuzz a list seed by deleting a random element

        Requires len(seed) >= 1"""
        delete_index = random.randint(0, len(seed) - 1)
        seed.pop(delete_index)
        return seed

    def _fuzz_one_element(self, seed):
        """Fuzz a list seed by fuzzing one element

        Requires len(seed) >= 1"""
        fuzz_index = random.randint(0, len(seed) - 1)
        randomizer = self._element_randomizer
        fuzzed_elem = randomizer.generate(seed=seed[fuzz_index])

        seed[fuzz_index] = fuzzed_elem
        return seed

    def _fuzz(self, seed):
        seed = self.eval_seed(seed)
        # Convert to list if needed. The thrift list type may be immutable
        if not isinstance(seed, list):
            seed = list(seed)
        if len(seed) == 0:
            # Seed is an empty list. The only valid fuzzer function
            # is the insert function
            fuzzed = self._fuzz_insert(seed)
        else:
            # All fuzzer functions are valid
            fuzz_fn = random.choice(
                [self._fuzz_insert, self._fuzz_delete, self._fuzz_one_element]
            )
            fuzzed = fuzz_fn(seed)
        return self.type_spec.construct_instance(fuzzed)

    def eval_seed(self, seed):
        return self.type_spec.construct_instance(
            [self._element_randomizer.eval_seed(e) for e in seed]
        )


class SetRandomizer(NonAssociativeContainerRandomizer):
    ttype = Thrift.TType.SET

    def _randomize(self):
        element_randomizer = self._element_randomizer

        length = self._get_length()
        length = min(length, element_randomizer.universe_size)

        elements = set()

        # If it is possible to get `length` unique elements,
        # in N = k*length iterations we will reach `length`
        # with high probability.
        i = 0
        k = 10
        N = k * length
        while len(elements) < length and i < N:
            element = element_randomizer.generate()
            if element is not None:
                elements.add(element)
            i += 1

        return self.type_spec.construct_instance(elements)

    def eval_seed(self, seed):
        return self.type_spec.construct_instance(
            {self._element_randomizer.eval_seed(e) for e in seed}
        )


class MapRandomizer(CollectionTypeRandomizer):
    ttype = Thrift.TType.MAP

    default_constraints = dict(CollectionTypeRandomizer.default_constraints)
    default_constraints.update({"key": {}, "value": {}})

    def _init_subrandomizers(self):
        subtypes = self.type_spec.get_subtypes()
        key_spec = subtypes[ThriftPyTypeSpec.SUBTYPE_KEY]
        val_spec = subtypes[ThriftPyTypeSpec.SUBTYPE_VALUE]

        key_constraints = self.constraints["key"]
        val_constraints = self.constraints["value"]

        self._key_randomizer = self.state.get_randomizer_for_spec(
            key_spec, key_constraints
        )
        self._val_randomizer = self.state.get_randomizer_for_spec(
            val_spec, val_constraints
        )

    def _randomize(self):
        key_randomizer = self._key_randomizer
        val_randomizer = self._val_randomizer

        length = self._get_length()
        length = min(length, key_randomizer.universe_size)

        elements = {}

        i = 0
        k = 10
        N = k * length
        while len(elements) < length and i < N:
            key = key_randomizer.generate()
            val = val_randomizer.generate()
            try:
                if key is not None and val is not None:
                    elements[key] = val
            except TypeError:
                # If we have a type error here it means that the key
                # can't be hashed. There can be structs that have
                # keys python doesn't like.
                #
                # For now just bail out.
                return self.type_spec.construct_instance(elements)
            i += 1

        return self.type_spec.construct_instance(elements)

    def eval_seed(self, seed):
        res = {}
        for key, val in six.iteritems(seed):
            key = self._key_randomizer.eval_seed(key)
            val = self._val_randomizer.eval_seed(val)
            res[key] = val
        return self.type_spec.construct_instance(res)


class StructRandomizer(BaseRandomizer):
    ttype = Thrift.TType.STRUCT

    default_constraints = dict(BaseRandomizer.default_constraints)
    default_constraints.update(
        {"p_include": 0.99, "max_recursion_depth": 3, "per_field": {}}
    )

    @property
    def universe_size(self):
        return INFINITY

    def _init_subrandomizers(self):
        subtypes = self.type_spec.get_subtypes()
        requiredness = self.type_spec.get_subtype_requiredness()

        field_rules = {}
        for name, field_spec in six.iteritems(subtypes):
            field_required = requiredness[name]
            field_constraints = self.constraints.get(name, {})

            field_randomizer = self.state.get_randomizer_for_spec(
                field_spec, field_constraints
            )

            field_rules[name] = {
                "required": field_required,
                "randomizer": field_randomizer,
            }
            field_rules[name].update(self.constraints["per_field"].get(name, {}))

        self._field_rules = field_rules
        self._is_union = self.type_spec.is_union
        self._field_names = list(self._field_rules)

    def _increase_recursion_depth(self):
        """Increase the depth in the recursion trace for this struct type.

        Returns:
        (is_top_level, max_depth_reached)

        If is_top_level is True, when decrease_recursion_depth is called
        the entry in the trace dictionary will be removed to indicate
        that this struct type is no longer being recursively generated.

        If max_depth_reached is True, the call to increase_recursion_depth
        has "failed" indicating that this randomizer is trying to generate
        a value that is too deep in the recursive tree and should return None.
        In this case, the recursion trace dictionary is not modified.
        """
        trace = self.state.recursion_trace
        name = self.type_name

        if name in trace:
            # There is another struct of this type higher up in
            # the generation tree
            is_top_level = False
        else:
            is_top_level = True
            trace[name] = self.constraints["max_recursion_depth"]

        depth = trace[name]

        if depth == 0:
            # Reached maximum depth
            if is_top_level:
                del trace[name]
            max_depth_reached = True
        else:
            depth -= 1
            trace[name] = depth
            max_depth_reached = False

        return (is_top_level, max_depth_reached)

    def _decrease_recursion_depth(self, is_top_level):
        """Decrease the depth in the recursion trace for this struct type.

        If is_top_level is True, the entry in the recursion trace is deleted.
        Otherwise, the entry is incremented.
        """
        trace = self.state.recursion_trace
        name = self.type_name

        if is_top_level:
            del trace[name]
        else:
            trace[name] += 1

    def _randomize(self):
        """Return randomized fields as a dict of {field_name: value}

        If fields cannot be generated due to an unsatisfiable
        constraint, return None.
        """
        (is_top_level, max_depth_reached) = self._increase_recursion_depth()
        if max_depth_reached:
            return None

        fields = {}
        fields_to_randomize = self._field_names
        p_include = self.constraints["p_include"]

        if self._is_union:
            if fields_to_randomize and random.random() < p_include:
                fields_to_randomize = [random.choice(fields_to_randomize)]
                p_include = 1.0
            else:
                fields_to_randomize = []

        for field_name in fields_to_randomize:
            rule = self._field_rules[field_name]
            required = rule["required"]

            field_p_include = rule.get("p_include", p_include)
            if not required and not (random.random() < field_p_include):
                continue

            value = rule["randomizer"].generate()

            if value is None:
                # Randomizer was unable to generate a value
                if required:
                    # Cannot generate the struct
                    fields = None
                    break
                else:
                    # Ignore the field
                    continue
            else:
                fields[field_name] = value

        self._decrease_recursion_depth(is_top_level)

        if (fields is None) or (self._is_union and not fields):
            return None
        else:
            if self._is_union:
                for f in self._field_names:
                    fields.setdefault(f, None)
            return self.type_spec.construct_instance(**fields)

    def _fuzz(self, seed):
        """Fuzz a single field of the struct at random"""
        fields = {}
        field_rules = self._field_rules
        seed = self.type_spec.value_to_dict(seed)
        if self._is_union:
            if not seed:
                # The seed could be malformed.
                # If that's the case just return none
                return None
            # The seed should be a single key/value pair
            field_name, seed_val = six.next(six.iteritems(seed))
            field_randomizer = field_rules[field_name]["randomizer"]
            fuzzed_val = field_randomizer.generate(seed=seed_val)
            fields[field_name] = fuzzed_val
            for f in self._field_names:
                fields.setdefault(f, None)
        elif field_rules:
            # Fuzz only one field and leave the rest with the seed value
            fuzz_field_name = random.choice(list(field_rules))
            fuzz_field_rule = field_rules[fuzz_field_name]
            fuzz_field_randomizer = fuzz_field_rule["randomizer"]
            fuzz_seed_val = seed.get(fuzz_field_name, None)
            fuzzed_value = fuzz_field_randomizer.generate(seed=fuzz_seed_val)

            if fuzzed_value is None:
                if fuzz_field_rule["required"]:
                    # Cannot generate the struct
                    return None
            else:
                fields[fuzz_field_name] = fuzzed_value

            for field_name, seed_val in six.iteritems(seed):
                if field_name == fuzz_field_name or field_name not in field_rules:
                    continue
                field_randomizer = field_rules[field_name]["randomizer"]
                fields[field_name] = field_randomizer.eval_seed(seed_val)

        return self.type_spec.construct_instance(**fields)

    def eval_seed(self, seed):
        fields = {}
        seed = self.type_spec.value_to_dict(seed)
        for key, val in six.iteritems(seed):
            if key not in self._field_rules:
                continue
            field_randomizer = self._field_rules[key]["randomizer"]
            val = field_randomizer.eval_seed(val)
            fields[key] = val

        if self._is_union:
            for f in self._field_names:
                fields.setdefault(f, None)
        return self.type_spec.construct_instance(**fields)


_ttype_to_randomizer = {}


def _init_types() -> None:
    # Find classes that subclass BaseRandomizer
    global_names = globals().keys()

    for name in global_names:
        value = globals()[name]
        if not isinstance(value, type):
            continue
        cls = value

        if issubclass(cls, BaseRandomizer):
            if cls.ttype is not None:
                _ttype_to_randomizer[cls.ttype] = cls


_init_types()


def _get_randomizer_class(type_spec):
    # Special case: i32 and enum have separate classes but the same ttype
    if type_spec.is_enum():
        return EnumRandomizer
    return _ttype_to_randomizer[type_spec.ttype]


class RandomizerState(object):
    """A wrapper around randomizer_map and recursion_trace

    All randomizers are initialized with a state. If a state is not explicitly
    specified, a clean one will be created. When randomizers create sub-
    randomizers, they should pass on their state object in order to share
    memoization and recursion trace information.

    --

    `randomizers` maps ttype to a list of already-constructed randomizer
    instances. This allows for memoization: calls to get_randomizer with
    identical arguments and state will always return the same randomizer
    instance.

    --

    `recursion_trace` maps a struct name to an int indicating the current
    remaining depth of recursion for the struct with that name.
    Struct randomizers use this information to bound the recursion depth
    of generated structs.
    If a struct name has no entry in the recursion trace, that struct
    is not currently being generated at any depth in the generation tree.

    When the top level randomizer for a struct type is entered, that
    randomizer's constraints are used to determine the maximum recursion
    depth and the maximum depth is inserted into the trace dictionary.
    At each level of recursion, the entry in the trace dictionary is
    decremented. When it reaches zero, the maximum depth has been reached
    and no more structs of that type are generated.

    --

    type_constraint_stacks maps type_name strings to
    constraint dictionaries that should be applied to all randomizers
    with type type_name. The items at the top of the stack
    (higher indices) were pushed most recently and thus override the
    constraints lower in the stack.

    Randomizer instances are responsible for pushing to and popping from
    their respective constraint stacks according to the type rules in
    their constraint dictionaries.
    """

    def __init__(self, default_constraints=None):
        self.randomizers = collections.defaultdict(list)
        self.recursion_trace = {}
        self.type_constraint_stacks = collections.defaultdict(list)
        self.default_constraints = default_constraints or {}

    def get_randomizer(self, ttype, spec_args, constraints):
        type_spec = get_spec(ttype, spec_args)
        return self.get_randomizer_for_spec(type_spec, constraints)

    def get_randomizer_for_spec(self, type_spec, constraints):
        """Get a randomizer object.
        Return an already-preprocessed randomizer if available and create a new
        one and preprocess it otherwise"""
        randomizer_class = _get_randomizer_class(type_spec)
        randomizer = randomizer_class(type_spec, self, constraints)

        # Check if this randomizer is already in self.randomizers
        randomizers = self.randomizers[randomizer.__class__.ttype]
        for other in randomizers:
            if other == randomizer:
                return other

        # No match. Register and preprocess this randomizer
        randomizers.append(randomizer)

        randomizer.preprocess()
        return randomizer

    def push_type_constraints(self, constraints):
        """Push type constraints onto the type constraint stack
        Return a list of stacks that need to be popped from

        Return `pushed`, a variable that should be passed back to
        pop_type_constraints when leaving the type constraints' scope.
        """

        pushed = []
        for key, val in six.iteritems(constraints):
            if key.startswith("|"):
                # This is a type constraint
                type_name = key[1:]
                stack = self.type_constraint_stacks[type_name]
                stack.append(val)
                pushed.append(stack)

        return pushed

    def pop_type_constraints(self, pushed):
        for stack in pushed:
            stack.pop()
