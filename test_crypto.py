#!/usr/bin/env python3

import hashlib
import os
import time


def intensive_crypto(duration=30):
    data = os.urandom(1024 * 1024)
    end = time.time() + duration
    while time.time() < end:
        hashlib.sha256(data).digest()
    print("Cryptographic operations completed")


if __name__ == "__main__":
    intensive_crypto()
