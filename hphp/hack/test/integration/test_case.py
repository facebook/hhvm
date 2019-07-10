# pyre-strict

import abc
import unittest
from typing import Generic, TypeVar


class TestDriver(abc.ABC, unittest.TestCase):
    @classmethod
    @abc.abstractmethod
    def setUpClass(cls, template_repo: str) -> None:
        raise NotImplementedError()

    @classmethod
    @abc.abstractmethod
    def tearDownClass(cls) -> None:
        raise NotImplementedError()

    @abc.abstractmethod
    def setUp(self) -> None:
        raise NotImplementedError()

    @abc.abstractmethod
    def tearDown(self) -> None:
        raise NotImplementedError()


T = TypeVar("T", bound=TestDriver)


class TestCase(abc.ABC, unittest.TestCase, Generic[T]):
    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/simple_repo"

    @classmethod
    @abc.abstractmethod
    def get_test_driver(cls) -> T:
        raise NotImplementedError()

    _test_driver: T

    @property
    def test_driver(self) -> T:
        return self._test_driver

    @classmethod
    def setUpClass(cls) -> None:
        cls._test_driver = cls.get_test_driver()
        cls._test_driver.setUpClass(cls.get_template_repo())

    @classmethod
    def tearDownClass(cls) -> None:
        cls._test_driver.tearDownClass()

    def setUp(self) -> None:
        self.test_driver.setUp()

    def tearDown(self) -> None:
        self.test_driver.tearDown()
