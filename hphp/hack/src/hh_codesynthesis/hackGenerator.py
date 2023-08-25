# pyre-strict
# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the "hack" directory of this source tree.
#
# A family of generators to create Hack constructs like class, function, interface, etc
#
# We are assuming all classes/interfaces/functions are properly defined before
# creating any implements/extends dependency. This is guaranteed by preceding
# step. In our case, it's the logic rule checked all constraints for us,
# therefore, the reasoning result from Clingo can be converted to code
# straightforward.
#
# _HackClassGenerator maintains each class definition.
# _HackInterfaceGenerator maintains each interface definition.
# _HackFunctionGenerator maintains each function definition.
# HackGenerator extends CodeGenerator combines all _Hack*Generator to
# emit Hack code on clingo output.
import functools
from collections import deque
from typing import Any, Dict, List, Optional, Set, Tuple

import clingo
from hphp.hack.src.hh_codesynthesis.codeGenerator import ClingoContext, CodeGenerator


class _HackBaseGenerator:
    """
    _HackBaseGenerator for the shared part of _HackInterfaceGenerator and
    _HackClassGenerator. In this case is the body of each class or interface
    definition. We are extending this to support method declaration/definition.
    """

    def __init__(self) -> None:
        super(_HackBaseGenerator, self).__init__()
        self.name = "Base"
        # A set of methods in this class/interface.
        self.methods: Set[str] = set()
        # A set of parameters invoked in dummy method.
        self.parameter_set: Set[str] = set()
        # A set of functions to invoke in dummy method.
        self.invoke_funcs_set: Set["_HackFunctionGenerator"] = set()
        # A set of parents this symbol had.
        self.parents: Set[str] = set()

    def add_method(self, method_name: str) -> None:
        self.methods.add(method_name)

    def add_parameter(self, parameter_type: str) -> None:
        self.parameter_set.add(parameter_type)

    def add_parent(self, parent_name: str) -> None:
        self.parents.add(parent_name)

    def _print_dummy_method_body(self) -> str:
        return ";"

    def _print_dummy_method(self) -> str:
        parameter_list = ", ".join(
            (map(lambda x: f"{x} ${x}_obj", sorted(self.parameter_set)))
        )
        if parameter_list == "" and len(self.invoke_funcs_set) == 0:
            return ""
        dummy_name = f"dummy_{self.name}_method"

        # We are defining a unique dummy_method among all the methods defined by
        # the user. If there is a naming conflict, simply extending it with "_".
        while dummy_name in self.methods:
            dummy_name += "_"
        return (
            f"\npublic function {dummy_name}({parameter_list}):"
            f" void{self._print_dummy_method_body()}\n"
        )

    def _print_method_body(self) -> str:
        return ";"

    def _print_method(self, method_name: str, static_keyword: str = " ") -> str:
        return (
            f"\npublic{static_keyword}function {method_name}():"
            f" void{self._print_method_body()}\n"
        )

    def _print_methods(self) -> str:
        return "".join(list(map(self._print_method, sorted(self.methods))))

    def _print_static_methods(self) -> str:
        return ""

    def _print_body(self) -> str:
        return (
            "{"
            + self._print_static_methods()
            + self._print_dummy_method()
            + self._print_methods()
            + "}"
        )


class _HackInterfaceGenerator(_HackBaseGenerator):
    """A generator to emit Hack Interface definition."""

    def __init__(self, name: str, **kwargs: Dict[str, Any]) -> None:
        super(_HackInterfaceGenerator, self).__init__()
        self.name = name
        # A set of extends relationship in this interface.
        self.extends: Set[str] = set()

    def add_extend(self, extend_from: str) -> None:
        self.extends.add(extend_from)

    def _print_extends(self) -> str:
        if len(self.extends) == 0:
            return ""
        return "extends {}".format(",".join(sorted(self.extends)))

    def __str__(self) -> str:
        return f"interface {self.name} {self._print_extends()} {self._print_body()}"


class _HackClassGenerator(_HackBaseGenerator):
    """A generator to emit Hack Class definition."""

    def __init__(self, name: str, **kwargs: Dict[str, Any]) -> None:
        super(_HackClassGenerator, self).__init__()
        self.name = name
        # Extend relationship could only be one parent class.
        self.extend: str = ""
        # A set of implements relationship in this class.
        self.implements: Set[str] = set()
        # A set of static methods in this class.
        self.static_methods: Set[str] = set()
        # A set of methods to invoke in dummy method.
        self.invoke_set: Set[Tuple[str, str]] = set()
        # A set of static methods to invoke in dummy method.
        self.invoke_static_set: Set[Tuple[str, str]] = set()

    def set_extend(self, extend_from: str) -> None:
        self.extend = extend_from

    def add_implement(self, implement: str) -> None:
        self.implements.add(implement)

    def add_static_method(self, method_name: str) -> None:
        self.static_methods.add(method_name)

    def add_invoke(self, object_type: str, method_name: str) -> None:
        if object_type in self.parameter_set:
            self.invoke_set.add((object_type, method_name))

    def add_invoke_static_method(self, class_name: str, method_name: str) -> None:
        self.invoke_static_set.add((class_name, method_name))

    def add_invoke_function(self, fn_obj: "_HackFunctionGenerator") -> None:
        self.invoke_funcs_set.add(fn_obj)

    def _print_extend(self) -> str:
        if self.extend == "":
            return ""
        return "extends {}".format(self.extend)

    def _print_implements(self) -> str:
        if len(self.implements) == 0:
            return ""
        return "implements {}".format(",".join(sorted(self.implements)))

    def _print_static_methods(self) -> str:
        return "".join(
            [self._print_method(x, " static ") for x in sorted(self.static_methods)]
        )

    def _print_dummy_method_body(self) -> str:
        return (
            "{"
            + "".join([f"\n${x[0]}_obj->{x[1]}();\n" for x in sorted(self.invoke_set)])
            + "".join(
                [f"\n{x[0]}::{x[1]}();\n" for x in sorted(self.invoke_static_set)]
            )
            + "".join(
                [
                    f"\n{x._print_callee()}\n"
                    for x in sorted(self.invoke_funcs_set, key=lambda x: x.name)
                ]
            )
            + "}"
        )

    def _print_method_body(self) -> str:
        return "{}"

    def __str__(self) -> str:
        return (
            f"class {self.name} {self._print_extend()} "
            + f"{self._print_implements()} {self._print_body()}"
        )


class _HackFunctionGenerator:
    """A generator to emit Hack Function definition."""

    def __init__(self, name: str, **kwargs: Dict[str, Any]) -> None:
        self.name = name
        # A set of static methods to invoke in the function.
        # A tuple with (class_name, static_method_name) added to the set.
        self.invoke_static_set: Set[Tuple[str, str]] = set()
        # A set of type(class/interface) methods to invoke in the function.
        # A tuple with (type_name, method_name) added to the set.
        self.type_method_set: Set[Tuple[str, str]] = set()
        # A set of functions to invoke in the function.
        self.invoke_funcs_set: Set["_HackFunctionGenerator"] = set()
        # A set of class objects to create in the function.
        # To invoke a class method, we are creating objects from this set
        # prior to invoke the method in `type_method_set`
        # A string with class_name added to the set.
        self.class_obj_set: Set[str] = set()
        # A list of parameter and argument pairs.
        # To invoke an interface method, we are adding this list to the function
        # parameter, the objects are created by the caller
        # later in the function body invokes method in `type_method_set`
        # A tuple with (parameter_type, argument_type) appended to the list.
        self.parameter_list: List[Tuple[str, str]] = []

    def add_invoke_static_method(self, class_name: str, method_name: str) -> None:
        self.invoke_static_set.add((class_name, method_name))

    def add_invoke_function(self, fn_obj: "_HackFunctionGenerator") -> None:
        self.invoke_funcs_set.add(fn_obj)

    def add_class_obj(self, class_name: str) -> None:
        self.class_obj_set.add(class_name)

    def add_class_method(self, class_name: str, method_name: str) -> None:
        self.type_method_set.add((class_name, method_name))

    def add_parameter(self, parameter_type: str, argument_type: str) -> None:
        self.parameter_list.append((parameter_type, argument_type))

    def _create_arguments(self) -> str:
        return "".join(
            [
                f"${argument_type}_obj = new {argument_type}();\n"
                for (_, argument_type) in self.parameter_list
            ]
        )

    def _print_parameters(self) -> str:
        return ", ".join(
            [
                f"{parameter_type} ${parameter_type}_obj"
                for (parameter_type, _) in self.parameter_list
            ]
        )

    def _print_arguments(self) -> str:
        return ", ".join(
            [f"${argument_type}_obj" for (_, argument_type) in self.parameter_list]
        )

    def _print_callee(self) -> str:
        return self._create_arguments() + f"{self.name}({self._print_arguments()});"

    def _print_body(self) -> str:
        return (
            "{"
            + "".join(
                [f"\n{x[0]}::{x[1]}();\n" for x in sorted(self.invoke_static_set)]
            )
            + "".join(
                [
                    f"\n{x._print_callee()}\n"
                    for x in sorted(self.invoke_funcs_set, key=lambda x: x.name)
                ]
            )
            + "".join(
                [
                    f"\n${class_name}_obj = new {class_name}();\n"
                    for class_name in sorted(self.class_obj_set)
                ]
            )
            + "".join(
                [
                    f"\n${type_name}_obj->{method}();\n"
                    for (type_name, method) in sorted(self.type_method_set)
                ]
            )
            + "}"
        )

    def __str__(self) -> str:
        return (
            f"function {self.name}({self._print_parameters()}): void"
            f" {self._print_body()}"
        )


class HackCodeGenerator(CodeGenerator):
    """A wrapper generator encapsulates each _Hack*Generator to emit Hack Code"""

    def __init__(
        self, solving_context: Optional[ClingoContext] = None, model_count: int = 1
    ) -> None:
        super(HackCodeGenerator, self).__init__(solving_context, model_count)
        self.class_objs: Dict[str, _HackClassGenerator] = {}
        self.interface_objs: Dict[str, _HackInterfaceGenerator] = {}
        self.function_objs: Dict[str, _HackFunctionGenerator] = {}

        # Sub set of class/interface objects are stub classes/interfaces.
        self.stub_classes: List[str] = []
        self.stub_interfaces: List[str] = []

    def _look_up_object_by_symbol(self, symbol: str) -> "_HackBaseGenerator":
        if symbol in self.class_objs:
            return self.class_objs[symbol]
        elif symbol in self.interface_objs:
            return self.interface_objs[symbol]
        raise RuntimeError("No object with symbol name {0}".format(symbol))

    def _add_class(self, name: str) -> None:
        self.class_objs[name] = _HackClassGenerator(name)

    def _add_interface(self, name: str) -> None:
        self.interface_objs[name] = _HackInterfaceGenerator(name)

    def _add_function(self, name: str) -> None:
        self.function_objs[name] = _HackFunctionGenerator(name)

    def _add_extend(self, name: str, extend: str) -> None:
        if name in self.class_objs:
            self.class_objs[name].set_extend(extend)
            self.class_objs[name].add_parent(extend)
        if name in self.interface_objs:
            self.interface_objs[name].add_extend(extend)
            self.interface_objs[name].add_parent(extend)

    def _add_implement(self, name: str, implement: str) -> None:
        if name in self.class_objs:
            self.class_objs[name].add_implement(implement)
            self.class_objs[name].add_parent(implement)

    def _add_method(self, name: str, method_name: str) -> None:
        if name in self.class_objs:
            self.class_objs[name].add_method(method_name)
        if name in self.interface_objs:
            self.interface_objs[name].add_method(method_name)

    def _add_static_method(self, name: str, method_name: str) -> None:
        if name in self.class_objs:
            self.class_objs[name].add_static_method(method_name)

    def _add_to_parameter_set(self, name: str, parameter_type: str) -> None:
        if name in self.class_objs:
            self.class_objs[name].add_parameter(parameter_type)
        elif name in self.interface_objs:
            self.interface_objs[name].add_parameter(parameter_type)

    def _add_invoke_function(self, name: str, function_name: str) -> None:
        # The function didn't purely pass as a name, we are passing a reference
        # to the function object. The reason is the function is going to have
        # parameter inside, so keep a reference to the function object make
        # code synthesis easier later.
        if function_name not in self.function_objs:
            return

        if name in self.class_objs:
            self.class_objs[name].add_invoke_function(self.function_objs[function_name])
        if name in self.function_objs:
            self.function_objs[name].add_invoke_function(
                self.function_objs[function_name]
            )

    def _add_object_in_function(self, name: str, class_name: str) -> None:
        if name in self.function_objs:
            self.function_objs[name].add_class_obj(class_name)

    def _add_invoke(self, name: str, object_type: str, method_name: str) -> None:
        if name in self.class_objs:
            self.class_objs[name].add_invoke(object_type, method_name)

    def _add_invoke_static_method(
        self, name: str, class_name: str, method_name: str
    ) -> None:
        if name in self.class_objs:
            self.class_objs[name].add_invoke_static_method(class_name, method_name)
        if name in self.function_objs:
            self.function_objs[name].add_invoke_static_method(class_name, method_name)

    def _add_invoke_in_function(
        self, name: str, type_name: str, method_name: str
    ) -> None:
        if name in self.function_objs:
            self.function_objs[name].add_class_method(type_name, method_name)

    def _add_parameter_to_function(
        self, name: str, parameter_type: str, argument_type: str
    ) -> None:
        if name in self.function_objs:
            self.function_objs[name].add_parameter(parameter_type, argument_type)

    def _find_stubs(self) -> None:
        for name, node in self.class_objs.items():
            if node.extend == "":
                self.stub_classes.append(name)

        for name, node in self.interface_objs.items():
            if len(node.extends) == 0:
                self.stub_interfaces.append(name)

    def validate_nodes(self) -> None:
        # Graph validation using the constraints specified in the context.
        assert (
            len(self.class_objs) >= self.solving_context.min_classes
        ), "Expected to get at least {0}, but only have {1} classes.".format(
            self.solving_context.min_classes, len(self.class_objs)
        )
        assert (
            len(self.interface_objs) >= self.solving_context.min_interfaces
        ), "Expected to get at least {0}, but only have {1} interfaces.".format(
            self.solving_context.min_interfaces, len(self.interface_objs)
        )
        assert (
            len(self.class_objs) + len(self.interface_objs)
            >= self.solving_context.number_of_nodes
        ), "Expected to get at least {0}, but only have {1} symbols.".format(
            self.solving_context.number_of_nodes,
            len(self.class_objs) + len(self.interface_objs),
        )

    def validate_stubs(self) -> None:
        # Check number of stub nodes.
        assert (
            len(self.stub_classes) >= self.solving_context.min_stub_classes
        ), "Expected to get at least {0}, but only have {1} stub classes.".format(
            self.solving_context.min_stub_classes, len(self.stub_classes)
        )
        assert (
            len(self.stub_interfaces) >= self.solving_context.min_stub_interfaces
        ), "Expected to get at least {0}, but only have {1} stub interfaces.".format(
            self.solving_context.min_stub_interfaces, len(self.stub_interfaces)
        )

    def validate_depth(self) -> None:
        # Validate the depth requirement (Union-find set).
        symbols = list(self.class_objs.keys()) + list(self.interface_objs.keys())
        depth: Dict[str, int] = dict.fromkeys(symbols, int(1))

        for symbol in symbols:
            ancestors = deque(
                [
                    (symbol, parent)
                    for parent in self._look_up_object_by_symbol(symbol).parents
                ]
            )
            while len(ancestors) != 0:
                (child, ancestor) = ancestors.popleft()
                depth[ancestor] = max(depth[ancestor], depth[child] + 1)
                ancestors.extend(
                    [
                        (ancestor, parent)
                        for parent in self._look_up_object_by_symbol(ancestor).parents
                    ]
                )
        assert (
            max(depth.values()) >= self.solving_context.min_depth
        ), "Expected to get at least {0} depth, but the max depth is {1}.".format(
            self.solving_context.min_depth, max(depth.values())
        )

    def validate_degree(self) -> None:
        in_degrees = [0] * 10
        # Iterate through class_objs and interface_objs.
        for node in self.class_objs.values():
            degree = len(node.implements)
            degree += 1 if node.extend != "" else 0
            in_degrees[degree] += 1
        for node in self.interface_objs.values():
            degree = len(node.extends)
            in_degrees[degree] += 1

        # Validate the degrees are greater than the specified deistribution.
        assert functools.reduce(
            lambda x, y: x and y,
            map(
                lambda actual, expected: actual >= expected,
                in_degrees,
                self.solving_context.degree_distribution,
            ),
            True,
        ), "Expected degree distribution {0}, but got {1}".format(
            self.solving_context.degree_distribution, in_degrees
        )

    def validate(self) -> bool:
        self.validate_nodes()
        self.validate_stubs()
        self.validate_depth()
        self.validate_degree()

        return True

    def __str__(self) -> str:
        return (
            "<?hh\n"
            + "\n".join(str(x) for x in self.class_objs.values())
            + "\n"
            + "\n".join(str(x) for x in self.interface_objs.values())
            + "\n"
            + "".join([str(x) + "\n" for x in self.function_objs.values()])
        )

    def on_model(self, m: clingo.Model) -> bool:
        # Same set of parameters and search algorithm will produce the same
        # result set. To make sure two different agent using the same settings
        # can produce different output, we are counting models in the result
        # set. The first agent using the same configuration gets first one,
        # the second agent using the same configuration gets second one, and so
        # on so forth.
        self.model_count -= 1
        if self.model_count > 0:
            return True

        # Separate into 'class(?)', 'interface(?)', 'funcs(?)',
        # 'implements(?, ?)', 'extends(?, ?)', 'add_method(?, ?)',
        # 'add_static_method(?, ?)', 'has_method_with_parameter(?, ?)'
        # 'invokes_function(?, ?)', 'creates_in_body(?, ?)'
        # 'invokes_in_method(?, ?, ?)', 'invokes_static_method(?, ?, ?)'
        # 'invokes_in_body(?, ?, ?)', 'has_parameter_and_argument(?, ?, ?)'
        predicates = m.symbols(atoms=True)
        node_func = {
            "class": self._add_class,
            "interface": self._add_interface,
            "funcs": self._add_function,
        }
        edge_func = {
            "extends": self._add_extend,
            "implements": self._add_implement,
            "add_method": self._add_method,
            "add_static_method": self._add_static_method,
            "has_method_with_parameter": self._add_to_parameter_set,
            "invokes_function": self._add_invoke_function,
            "creates_in_body": self._add_object_in_function,
        }
        trip_func = {
            "invokes_in_method": self._add_invoke,
            "invokes_static_method": self._add_invoke_static_method,
            "invokes_in_body": self._add_invoke_in_function,
            "has_parameter_and_argument": self._add_parameter_to_function,
        }
        # Three passes,
        #   First pass creates individual nodes like class, interface, function.
        for predicate in predicates:
            if predicate.name in node_func:
                node_func[predicate.name](predicate.arguments[0].string)

        #   Second pass creates edge between two nodes.
        for predicate in predicates:
            if predicate.name in edge_func:
                edge_func[predicate.name](
                    predicate.arguments[0].string, predicate.arguments[1].string
                )
        #   Third pass creates relationships between three nodes.
        for predicate in predicates:
            if predicate.name in trip_func:
                trip_func[predicate.name](
                    predicate.arguments[0].string,
                    predicate.arguments[1].string,
                    predicate.arguments[2].string,
                )
        self._find_stubs()
        return False
