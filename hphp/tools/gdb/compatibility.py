"""
I don't understand Python's version incompatibilities and also Python sucks.
"""
# @lint-avoid-python-3-compatibility-imports
# @lint-avoid-pyflakes3
# @lint-avoid-pyflakes2

# I've heard that it's "Pythonic" or something to use try/catch for everything.
# Most likely people who think this should be hospitalized, but who am I to
# argue.  I'm just scared for my life.

try: xrange
except NameError:
    xrange = range

try: unicode
except NameError:
    unicode = str

try: reduce
except NameError:
    from functools import reduce

try:
    from itertools import izip
except ImportError:
    izip = zip
