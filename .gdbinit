# File: .gdbinit
# Author: Derek Huang
# Brief: xcp-revisited local GDB init file
# Copyright: MIT License

# enable printing derived object type from base pointer using vtable info
set print object
# start Python to load pretty printers
python
from gdb import current_objfile
from gdb.printing import register_pretty_printer
from pathlib import Path
import sys
# append src to module search path and import printer modules
sys.path.insert(0, str(Path.cwd() / "src"))
from pdxcp_fruit.pprinter import pretty_printer as fruit_printer
# register pdxcp_fruit printers
register_pretty_printer(current_objfile(), fruit_printer())
end
