# Stockholm

### **Project Overview**

1. **Objective**:
    - Develop a ransomware simulation that encrypts files in a specific directory (`./infection`) and offers a decryption method.
    - The project aims to teach encryption, file handling, and basic cybersecurity concepts.
2. **Key Features**:
    - **Encryption**: The program must encrypt files within the `./infection` directory using a symmetric encryption algorithm like Fernet.
    - **Decryption**: Provide an option to decrypt the files using a stored key.
    - **Key Management**: The key used for encryption should be saved securely in a file and used for decryption.
3. **Command-Line Interface**:
    - `r`, `-reverse`: Reverse the encryption process using a provided key.
    - `s`, `-silent`: Encrypt files without displaying their names.
    - `v`, `-version`: Show the program version.
    - `h`, `-help`: Display usage instructions.
4. **Encryption Details**:
    - Encrypt only files matching certain extensions (e.g., `.txt`, `.jpg`).
    - Files without extensions are usually not encrypted to simulate real-world ransomware behavior.
5. **Decryption**:
    - The decryption process requires the original encryption key.
    - If the correct key is not provided, the program should not decrypt the files.
6. **User Interaction**:
    - The program should display a message indicating that the files have been encrypted and how to obtain the decryption key, mimicking a ransom note.
    - When the encryption is completed, display a warning message to simulate the scare factor.
7. **Security Considerations**:
    - The use of secure encryption methods.
    - Proper error handling for missing files or incorrect keys.
    - Implementation of optional silent mode to suppress output.

### Sources

https://www.youtube.com/watch?v=Sv8yu12y5zM

https://thepythoncode.com/article/encrypt-decrypt-files-symmetric-python
