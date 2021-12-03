# pyre-strict

import abc
import os
import unittest
from typing import Generic, Optional, TypeVar


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


class TestCase(unittest.TestCase, Generic[T]):
    @classmethod
    def get_template_repo(cls) -> str:
        return "hphp/hack/test/integration/data/simple_repo"

    @classmethod
    def get_test_driver(cls) -> T:
        raise NotImplementedError()

    _test_driver: Optional[T] = None

    @property
    def test_driver(self) -> T:
        test_driver = self._test_driver
        assert test_driver is not None
        return test_driver

    @classmethod
    def setUpClass(cls) -> None:
        cls._test_driver = cls.get_test_driver()
        cls._test_driver.setUpClass(cls.get_template_repo())

    @classmethod
    def tearDownClass(cls) -> None:
        test_driver = cls._test_driver
        assert test_driver is not None
        test_driver.tearDownClass()

    def setUp(self) -> None:

        # These scripts assume working directory fbcode.
        cwd = os.path.basename(os.getcwd())
        if cwd == "fbcode":
            self.undo_chdir = None
            pass  # buck
        elif cwd == "fbsource":
            self.undo_chdir = os.getcwd()
            os.chdir("fbcode")  # buck2
        else:
            raise RuntimeError("Invalid working directory")

        self.test_driver.setUp()

    def tearDown(self) -> None:
        if self.undo_chdir is not None:
            # If we chdir then TPX/Buck2 get confused, so make sure we
            # put things back to how they were.
            # T107261961 to fix this on the TPX/Buck2 side.
            os.chdir(self.undo_chdir)
            self.undo_chdir = None
        self.test_driver.tearDown()
