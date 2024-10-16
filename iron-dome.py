#!/usr/bin/env python3

import sys
import os
import ctypes
from ctypes import *


def send_args(files):
    clibrary = ctypes.CDLL("./clibnotify.so")

    # Conver the Python string lsit to bytes
    files_in_bytes = [file.encode('utf-8') for file in files]

    # Create an array of char pointers
    arr = (c_char_p * len(files_in_bytes))(*files_in_bytes)

    # Pass the array to the C function
    clibrary.check_functions(len(files_in_bytes), arr)
    print("Files scanned")


if __name__ == '__main__':
    files = []
    if os.getuid() == 0:
        print(f"len of sys args: {len(sys.argv)}")
        if len(sys.argv) < 2:
            print("No arguments provided, scanning /etc, /var/log, /home")
            files.append("/etc")
            files.append("/var/log")
            files.append("/home")
            send_args(files)
        else:
            print("Arguments provided, scanning the provided directories")
            for arg in sys.argv[:]:
                if os.path.isdir(arg):
                    files.append(arg)
                elif os.path.isfile(arg):
                    files.append(arg)
                print(arg)
            send_args(files)
    else:
        print('You need to be root to run this script')
        sys.exit(1)
