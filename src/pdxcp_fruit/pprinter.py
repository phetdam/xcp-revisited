"""GDB pretty printers for pdxcp_fruit.

.. codeauthor:: Derek Huang <djh458@stern.nyu.edu>
"""

# NB: gdb module exists only when GDB is running
import gdb
from gdb.printing import RegexpCollectionPrettyPrinter


class CandyApplePrinter:
    """GDB pretty-printer class for the ``pdxcp::candy_apple`` class.

    .. note::

       More recent versions of GDB don't have the ``gdb.ValuePrinter``.
    """

    def __init__(self, val: gdb.Value):
        """Initialize the pretty-printer.

        This follows the ``ValuePrinter`` example from the GDB docs.
        """
        self.__val = val

    def to_string(self) -> str:
        """Return the pretty string object representation GDB should print."""
        return f"pdxcp::candy_apple(weight={self.__val['weight_']})"


class FruitPrinter:
    """GDB pretty-printer class for the ``pdxcp::fruit`` conrete base class."""

    def __init__(self, val: gdb.Value):
        """Initialize the pretty-printer.

        .. note::

           Is the attribute mangling necessary?
        """
        self.__val = val

    def to_string(self) -> str:
        """Return the pretty string object representation GDB should print."""
        return (
            f"pdxcp::fruit(weight={self.__val['weight_']}, "
            f"kcal_per_oz={self.__val['kcal_per_oz_']})"
        )


class ApplePrinter:
    """GDB pretty-printer class for the ``pdxcp::apple`` class."""

    def __init__(self, val: gdb.Value):
        """Initialize the pretty-printer."""
        self.__val = val

    def to_string(self) -> str:
        """Return the pretty string object representation GDB should print."""
        return f"pdxcp::apple(weight={self.__val['weight_']})"


class BananaPrinter:
    """GDB pretty-printer class for the ``pdxcp::banana`` class."""

    def __init__(self, val: gdb.Value):
        """Initialize the pretty-printer."""
        self.__val = val

    def to_string(self) -> str:
        """Return the pretty string object representation GDB should print."""
        return f"pdxcp::banana(weight={self.__val['weight_']})"


def pretty_printer() -> RegexpCollectionPrettyPrinter:
    """Return a pretty printer for the ``pdxcp_fruit`` classes.

    The ``RegexpCollectionPrettyPrinter`` used selects the correct printer class
    based on regular expression matching of the fully-qualified type name.
    """
    pp = RegexpCollectionPrettyPrinter("pdxcp_fruit")
    pp.add_printer("candy_apple", r"^pdxcp::candy_apple$", CandyApplePrinter)
    pp.add_printer("fruit", r"^pdxcp::fruit$", FruitPrinter)
    pp.add_printer("apple", r"^pdxcp::apple$", ApplePrinter)
    pp.add_printer("banana", r"^pdxcp::banana$", BananaPrinter)
    return pp
