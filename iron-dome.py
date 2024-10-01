import sys
import os
import ctypes


def send_args(files, folders):
    clibrary = ctypes.CDLL("./clibnotify.so")
    for file in files:
        print('Checking file:', file)
        if not os.path.exists(file):
            print('File does not exist')
            continue
        clibrary.notify(file.encode('utf-8'), 0)

    for folder in folders:
        print('Checking folder:', folder)
        if not os.path.exists(folder):
            print('Folder does not exist')
            continue
        clibrary.notify(0, folder.encode('utf-8)'))


if __name__ == '__main__':
    files = []
    folders = []
    if os.getuid() == 0:
        if len(sys.argv) < 2:
            folders.append("/etc")
            folders.append("/var/log")
            folders.append("/home")
            send_args(files, folders)
        for arg in sys.argv[1:]:
            if os.path.isdir(arg):
                folders.append(arg)
            elif os.path.isfile(arg):
                files.append(arg)
            print(arg)
        send_args(files, folders)
    else:
        print('You need to be root to run this script')
        sys.exit(1)
