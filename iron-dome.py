#!/usr/bin/env python3

import sys
import os
import ctypes
import argparse
from ctypes import *


def send_args(files, backup=None):
    try:
        clibrary = ctypes.CDLL("./clibnotify.so")
    except OSError:
        print("Error loading the shared library")
        sys.exit(1)

    # Conver the Python string lsit to bytes
    files_in_bytes = [file.encode('utf-8') for file in files]

    # Create an array of char pointers
    arr = (c_char_p * len(files_in_bytes))(*files_in_bytes)

    # Pass the array to the C function
    clibrary.check_functions(len(files_in_bytes), arr, backup)
    print("Files scanned")


if __name__ == '__main__':
    files = []
    parser = argparse.ArgumentParser(description='Iron Dome Monitoring Script')
    parser.add_argument('files', nargs='*', default=['/etc', '/var/log', '/home'], help='Files or directories to scan')
    parser.add_argument('-b',
                        '--backup',
                        type=int,
                        help='Time in seconds for the backup interval')
    args = parser.parse_args()
    if os.getuid() == 0:
        if len(args.files) == 0:
            print("No arguments provided, scanning /etc, /var/log, /home")
            files.append("iron-dome.py")
            files.append("/etc")
            files.append("/var/log")
            files.append("/home")
            send_args(files)
        else:
            print("Arguments provided, scanning the provided directories")
            files.append("iron-dome.py")
            for arg in args.files:
                if os.path.isdir(arg):
                    files.append(arg)
                elif os.path.isfile(arg):
                    files.append(arg)
                else:
                    print("Invalid argument: ", arg)
            if len(files) == 1 and files[0] != "iron-dome.py":
                print("No valid files to scan")
                sys.exit(1)
            send_args(files, args.backup)
    else:
        print('You need to be root to run this script')
        sys.exit(1)
