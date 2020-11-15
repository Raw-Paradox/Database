'''
This is a wrapper for bplus.so
'''

import ctypes
import pathlib
from typing import Tuple


class key_t(ctypes.Structure):
    _fields_ = [("key", ctypes.c_int32), ]


class row_t(ctypes.Structure):
    _fields_ = [("id", ctypes.c_int32),
                ("username", ctypes.c_char * 32),
                ("email", ctypes.c_char * 255)]


libname = "tree.so"
c_lib = ctypes.CDLL(libname)
c_lib.open_db.restype = ctypes.c_void_p


class Database:
    def __init__(self, name):
        self.table = c_lib.open_db(ctypes.c_char_p(name.encode('utf-8')))

    def close_db(self):
        c_lib.close_db(ctypes.c_void_p(self.table))

    def print_table(self):
        c_lib.print_table(ctypes.c_void_p(self.table))

    def insert(self, *args):
        print(f'insert {args}')
        c_lib.insert(ctypes.c_void_p(self.table), key_t(args[0]), row_t(args[0], args[1].encode('utf-8'), args[2].encode('utf-8')))
