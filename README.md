**DemoVideo**: _https://youtu.be/YUA85d7KCy0_
| File Name | Link | Purpose of the file |
| :---- | :---- | :---- |
| DemoVideo formative | [**DemoVideo**](https://youtu.be/YUA85d7KCy0)  | complete explanation of the entire project , and complete guard on how to run it | 
| Blockchain.c | [**blockchain.c**](https://github.com/josep-prog/formative_introduction_blockchain/blob/main/src/blockchain.c)  | I created this file to manage the blockchain. It creates the genesis, borrowing and returning blocks, links them using hashes, checks active loans, validates the blockchain (hashes, links and signatures), and saves and loads the chain file. |
| Blockchain.h | [**blockchain.h**](https://github.com/josep-prog/formative_introduction_blockchain/blob/main/src/blockchain.h)  | I created this file to define the Block structure and declare the blockchain functions so that other files, especially main.c, can use them. |
| Crypto.c | [**crypto.c**](https://github.com/josep-prog/formative_introduction_blockchain/blob/main/src/crypto.c)  | I created this file to handle the security part of the system. It generates keys, saves and loads the passphrase-encrypted key file, creates and verifies digital signatures, and hashes librarian PINs using OpenSSL. |
| Crypto.h | [**crypto.h**](https://github.com/josep-prog/formative_introduction_blockchain/blob/main/src/crypto.h)  | I created this file to declare the cryptographic functions so that main.c and blockchain.c can use the security functions from crypto.c |
| Main.c | [**main.c**](https://github.com/josep-prog/formative_introduction_blockchain/blob/main/src/main.c)  | I created this file to control the whole program. It handles the menu and calls the other files when the program needs to load records, create blockchain transactions, or perform security checks. |
| Registry.h | [**registry.h**](https://github.com/josep-prog/formative_introduction_blockchain/blob/main/src/registry.h)   | I created this file to define the Book, Member and Librarian structures and declare the registry functions used by main.c. |
| Registry.c | [**registry.c**](https://github.com/josep-prog/formative_introduction_blockchain/blob/main/src/registry.c)  | I created this file to load the books, members and librarians from the data files, report any bad lines, and find a record by ID. |
| Books.txt | [**books.txt**](https://github.com/josep-prog/formative_introduction_blockchain/blob/main/data/books.txt)  | This file is for storing the registered books, including their IDs, titles, and authors. |
| Members.txt | [**members.txt**](https://github.com/josep-prog/formative_introduction_blockchain/blob/main/data/members.txt)  | this file to store the registered library members and their basic information. |
| Librarians.txt | [**librarians.txt**](https://github.com/josep-prog/formative_introduction_blockchain/blob/main/data/librarians.txt) | this file stores the staff who can log in: their ID, name, role (ADMIN or LIBRARIAN) and a PBKDF2 hash of their PIN. |
| Makefile | [**Makefile**](https://github.com/josep-prog/formative_introduction_blockchain/blob/main/Makefile) | this file to make compiling the whole project easier by providing the commands needed to build the program and link OpenSSL. |

# **How My Blockchain-Based Library Lending Tracker Works**

A traditional library lending system can be as simple as keeping a list of which books have been borrowed and returned. However, such a record can be changed after the transaction has happened. For example, someone could change a record from “borrowed” to “returned” without leaving an obvious sign that the record was changed. The purpose of my project was to create a small library lending system where such changes could be detected. To achieve this, I used a simple blockchain written in C. Instead of keeping each lending action as an independent record, every borrow and return is stored as a block. The blocks are connected to each other using their digital fingerprints, called hashes. If an older record is changed, its fingerprint also changes, which allows the program to detect that the chain has been altered. This approach follows the main purpose of the assignment, which requires a blockchain structure, SHA-256 hashing, digital signatures, validation, and a simple interface for managing lending records.

## **Approach for Entire project**

<img width="916" height="606" alt="structure" src="https://github.com/user-attachments/assets/e85e3837-be68-4c46-9ee4-873c61da14c0" />

I divided the program into different parts so that each part has a clear responsibility. The main.c file controls what the user sees and connects the other parts of the program. The registry.c file is responsible for loading and searching for books and members. The blockchain.c file contains the main blockchain logic, such as creating blocks, connecting them, checking borrowing status, calculating hashes, and validating the chain. The crypto.c file handles the digital signatures and the creation of the cryptographic key. This separation makes the program easier to understand because the code that deals with books and members is kept separate from the code that deals with the blockchain and cryptography.

**Run the project**

1. Clone : git clone [https://github.com/josep-prog/formative\_introduction\_blockchain.git](https://github.com/josep-prog/formative_introduction_blockchain.git)   
2. cd  formative\_introduction\_blockchain

| Command  | Tests |
| :---- | :---- |
| sudo apt install gcc make libssl-dev | Is for installing the C compiler , build tool and openSSL development files and the program needs |
| make clean && make | Deletes the old program file and compiles all the source files again into the new program called library |
| ./library  | Starts the program. It asks for the key passphrase and a librarian login, then shows the menu |
| ./library --hash-pin LIB003 4321 | Prints the PIN hash to put in a new line of data/librarians.txt |

The signing key in data/key.pem is encrypted with a passphrase (at least 4 characters). The program asks for it at start-up, or reads it from the `LIBRARY_KEY_PASSPHRASE` environment variable. The first run creates the key with whatever passphrase you give, and later runs must use the same one.

If you ran an older version of this program, delete the old files first, because the chain format changed: `rm -f data/chain.txt data/key.pem`

**Librarians (logins):**

| ID | Name | Role | PIN |
| :---- | :---- | :---- | :---- |
| LIB001 | Joseph Nishimwe | ADMIN | 1234 |
| LIB002 | Librarian | LIBRARIAN | 5678 |
<img width="1272" height="704" alt="librarian-admin" src="https://github.com/user-attachments/assets/f98a7e52-d0d0-489c-934f-130c141d665e" />


After 3 wrong attempts the program prints "Access denied." and exits. Only an ADMIN can run the tamper-detection demo.

**Books**  : 

| ID | Title  | Author |
| :---- | :---- | :---- |
| BK001 | The Money Trap: Lost Illusions Inside the Tech Bubble | Alok Sama |
| BK002 | Wars Guns & Votes: Democracy in Dangerous Places | Paul Collier |
| BK003 | Sustainable Leadership | Clarke Murphy |
| BK004 | Ancient Philosophy: The Fundamentals | Daniel W. Graham |
| BK005 | Gulliver's Travels and Other Writings | Jonathan Swift  |

**Member:** 

| ID | Name | Course |
| :---- | :---- | :---- |
| ALU001 | Irakoze Jean | BSE |
| ALU002 | Joseph Habimana | BSE |
| ALU003 | Uwase Diane | BEL |
| ALU004 | Manzi Eric | BSE |
| ALU005 | Mukamana Alice | BEL |

**What to expect :** 

1\. On start-up the program prints "Loaded 5 books, 5 members and 2 librarians.", unlocks (or creates) the encrypted signing key in data/key.pem, loads the saved blockchain from data/chain.txt (or starts a new one with the genesis block), and asks the librarian to log in.

<img width="1918" height="561" alt="1" src="https://github.com/user-attachments/assets/e52db112-11af-441c-9ab0-adbfbd9ab9b4" />

2\. A menu with seven options appears: borrow, return, view records, validate, mark overdue loans, tamper demo and exit.

<img width="1920" height="1080" alt="2" src="https://github.com/user-attachments/assets/84364e74-133d-4b7c-9c63-df924bc6caec" />

3\. Every successful borrow or return is added as a signed block and saved to data/chain.txt straight away, so the records are still there the next time the program starts. Each block records which librarian made it. Unknown IDs, a book that is already on loan, a return for a book that is not on loan, or a return by a member who did not borrow the book print an ERROR and add nothing.

<img width="1920" height="1080" alt="3" src="https://github.com/user-attachments/assets/1a028929-8354-4b43-b17a-e216dcc9ab8f" />

4\. Option 4 (validate) prints "Blockchain is VALID" for an untouched chain and "Blockchain is INVALID - tampering detected!" followed by the block number and the reason (bad hash, broken link, bad signature, bad index or bad genesis block). Option 5 adds an OVERDUE block for every loan older than the loan period. Option 6 (ADMIN only) tampers with Block \#1 in memory only, and the file on disk is never overwritten with an invalid chain.

<img width="1920" height="1080" alt="4" src="https://github.com/user-attachments/assets/275da27b-51ee-41cd-8b2f-796498d14335" />

## **Books and Members**

Before the system can record any lending activity, it first loads the book and member information from two files: books.txt and members.txt. The assignment requires these files to be loaded when the program starts and used to check whether the supplied book and member IDs are valid.

The book file contains the book ID, title, and author, while the member file contains the member ID, full name, and course code. The program stores these records in simple arrays while it is running. The functions load\_books() and load\_members() read the two files line by line. A line with missing fields, a field that is too long, a duplicate ID or a '|' character is reported with its line number and skipped. Windows (CRLF) line endings are handled. If either file is missing or has no valid records, the program reports an error and stops. A third file, librarians.txt, is loaded the same way by load\_librarians(). This is important because the system should not record a lending transaction if it cannot confirm that the book or member exists.

When a user wants to borrow a book, the program uses find\_book() to search for the book ID and find\_member() to search for the member ID. If either one cannot be found, the transaction is rejected. The comparison is also exact, meaning that an ID such as BK001EXTRA cannot be treated as the same as BK001. In this way, the registry acts as the first level of protection before anything is added to the blockchain.

**Testing :** 

The blockchain is saved between runs, so run `rm -f data/chain.txt` before each command below to start from a clean chain. First set the passphrase once with `export LIBRARY_KEY_PASSPHRASE=demo-pass`. Every command starts by logging in (`LIB001` / `1234`).

| *command* | *purpose* |
| :---- | :---- |
| printf 'LIB001\\n0000\\nLIB001\\n0000\\nLIB001\\n0000\\n' \| ./library | Enters a wrong PIN three times, which checks that the program prints "Access denied." and exits. |
| printf 'LIB001\\n1234\\n1\\nBK001\\nALU001\\n7\\n' \| ./library | Borrows book BK001 for ALU001 and then exits, which checks that a normal borrow works. |
| printf 'LIB001\\n1234\\n1\\nBK001\\nALU001\\n1\\nBK001\\nALU002\\n7\\n' \| ./library | Borrows BK001 for one member and then tries to borrow the same book for another member, which checks that the program refuses a book that is already on loan. |
| printf 'LIB001\\n1234\\n1\\nBK999\\nALU001\\n1\\nBK001\\nALU999\\n7\\n' \| ./library | Tries to borrow an unknown book and then uses an unknown member, which checks that both are rejected with "ERROR: Book or Member not found". |
| printf 'LIB001\\n1234\\n1\\nBK001\\nALU001\\n2\\nBK001\\nALU002\\n2\\nBK001\\nALU001\\n7\\n' \| ./library | Borrows a book, tries to return it as the wrong member, then returns it as the borrower, which checks that only the borrower can return it. |
| printf 'LIB001\\n1234\\n2\\nBK001\\nALU001\\n7\\n' \| ./library | Tries to return a book that was never borrowed, which checks that the program prints an error. |
| printf 'LIB001\\n1234\\n1\\nBK001\\nALU001\\n5\\n3\\n7\\n' \| LOAN\_PERIOD\_SECONDS=0 ./library | Borrows a book with a loan period of 0 seconds and marks overdue loans, which checks that an OVERDUE block is added and shown with a VALID signature. |
| printf 'LIB001\\n1234\\n1\\nBK001\\nALU001\\n6\\n4\\n7\\n' \| ./library | Borrows a book, runs the tamper demo, and then validates again, which checks that tampering is detected and stays detected. |
| printf 'LIB002\\n5678\\n6\\n7\\n' \| ./library | Logs in as a LIBRARIAN and tries the tamper demo, which checks that only an ADMIN can run it. |
| printf 'LIB001\\n1234\\nabc\\n7\\n' \| ./library | Types letters where a number is expected, which checks that the program ignores bad menu input and does not crash. |
| printf '' \| ./library | Sends no input at all, which checks that the program exits cleanly and does not hang when the input closes. |


**What to expect :** 

1\. <img width="1920" height="1080" alt="1" src="https://github.com/user-attachments/assets/525eacc9-b608-4172-a64e-fad79f377514" />



2\. <img width="1920" height="1080" alt="2" src="https://github.com/user-attachments/assets/e8649ab8-aa9b-4167-88a8-a8a881c32cd4" />



3\.<img width="1210" height="495" alt="3" src="https://github.com/user-attachments/assets/922f0c14-7ff0-43dd-ad07-5de445488964" />

 
## **The Blockchain and the Block**

The main idea of the system is that every important lending event becomes a block. A block contains information such as the book, the member, the action taken, the librarian who recorded it, the time of the action, the previous block's hash, a digital signature, and its own hash. The assignment requires the block to contain these important pieces of information so that each lending event can be properly recorded.

For example, when a member borrows a book, the program creates a block containing the book ID and title, the member ID and name, and the action BORROWED. When the book is returned, another block is created with the action RETURNED. If a loan runs past the loan period (14 days by default), option 5 adds an OVERDUE block; the book stays on loan until it is returned. I also copy the book title and member name into the block at the time of the transaction. This means that the block keeps the information that was recorded at that particular moment instead of depending on the registry later.

The first block is called the **genesis block**. It does not represent a real borrowing or returning action. Its purpose is simply to start the blockchain. Its previous hash is set to 64 zeros because there is no block before it. Every other block then stores the hash of the block immediately before it. The assignment specifically requires this genesis block and the connection between blocks through their previous hashes.

This creates a chain such as:

Genesis →Borrow → Return → Borrow → Return

The important point is that a block does not stand alone. It remembers the previous block, which makes changes to earlier records easier to detect.

## **Hashing and Detecting Changes**

A hash can be understood as a digital fingerprint of information. In my program, SHA-256 is used to create this fingerprint. Before calculating the hash, the program puts the important information from the block into one consistent piece of data. The information includes the block number, time, book, member, librarian, action, and previous hash. The signature information is also included when the final block hash is calculated.

The function create\_transaction\_data() was created for this purpose. Its job is simply to prepare the important information from a block in one consistent format. This is useful because the same information needs to be used when creating, signing, and checking a transaction.

The calculate\_hash() function then takes this information and creates the block's SHA-256 fingerprint. When a block is created, the fingerprint is stored inside the block. Later, when the blockchain is checked, the program calculates the fingerprint again and compares it with the stored one. If the two are different, the contents of the block have changed.

For example, suppose a block originally contains the title “Things Fall Apart.” If someone changes it to “TAMPERED TITLE,” the block's fingerprint will no longer be the same. This gives the system a simple way to detect that the record has been changed.

## **Digital Signatures**

In addition to hashing, the program uses digital signatures for lending transactions. A digital signature can be thought of as a special digital stamp attached to a transaction. The first time the program runs, it creates a pair of cryptographic keys and saves them in data/key.pem, encrypted with AES-256 under a passphrase and readable only by the owner. On later runs the same key is unlocked with the passphrase, so old signatures can still be checked. A copy of the file alone is not enough to sign blocks. The private key is used to sign borrow and return transactions, while the other part of the key pair is used to check the signature later.

The function generate\_key\_pair() creates this key pair, and save\_key() and load\_key() store and read it. If an older, unencrypted key file is found, it is saved again in encrypted form. The function sign\_data() creates a signature for a lending transaction, and verify\_signature() checks whether the signature is still valid. When the user chooses to view the records, the program checks the signatures and reports whether they are valid or invalid. validate\_chain() checks them too.

The purpose of using signatures here is to provide another way of checking the lending records. Hashing helps the program detect changes in the data, while the signature provides a way to check the authenticity of the transaction. The assignment specifically requires digital signatures to authenticate lending actions. Because the librarian's ID is part of the signed data, a signature also proves which logged-in librarian recorded the action.

## **Authentication and Access Control**

Before the menu appears, a librarian must log in with their ID and PIN. PINs are never stored. data/librarians.txt holds a PBKDF2-HMAC-SHA256 hash of each PIN (100,000 rounds, salted with the librarian ID), and the check uses a constant-time comparison. The PIN is not shown while typing. After three failed attempts, the program exits. Each librarian has a role: a LIBRARIAN can borrow, return, view, validate and mark overdue loans, while only an ADMIN can run the tamper-detection demo.

## **Borrowing a Book**

The borrowing process follows a simple sequence. First, the user enters a book ID and a member ID. The program checks both IDs against the registries. If either ID is unknown, the program stops the transaction and displays an error.

If both IDs are valid, the program then checks whether the book is already on loan. For this, I created the find\_active\_borrow() function. Instead of keeping a separate list of borrowed books, this function looks through the blockchain from the newest record backwards. It finds the latest transaction involving that book.

If the latest transaction says BORROWED, the book is currently out. If the latest transaction says RETURNED, the book is available. This means that the blockchain itself is used to determine the current state of the book.

If the book is available, the program calls create\_lending\_block() with the action BORROWED. This creates a new block, connects it to the previous block, signs the transaction, calculates its hash, and adds it to the end of the blockchain. This follows the assignment's required process of checking the IDs and the book's loan status before creating a new borrowing record.

## **Returning a Book**

The return process is similar. The user provides the book ID and the member ID, and the program confirms that both exist in the registries. It then uses find\_active\_borrow() to look for the book's most recent borrowing record.

If there is no active borrowing record, the program does not create a return block. This prevents someone from returning a book that the system does not consider borrowed.

If the book is currently borrowed, the member ID must match the member who borrowed it; otherwise the return is refused. The program then creates a new RETURNED block with the member information from the original borrowing block. The new block is then signed, given its hash, and added to the chain. This follows the assignment requirement for handling returns.

## **Validating the Blockchain**

One of the most important functions in the program is validate\_chain(). Its purpose is to answer a simple question: **Has anything in the blockchain been changed?**

The function first checks that every block's index matches its position, and that the genesis block has the GENESIS action and 64 zeros as its previous hash. It then checks every block in three ways. First, it calculates the block's hash again and compares it with the hash already stored in that block. If they are different, the contents of the block have changed. Second, for every block after the first one, it checks whether the block's previous\_hash still matches the actual hash of the block before it.

Third, for every block after the genesis block, it verifies the digital signature with the public key. This catches a block that was rewritten and re-hashed but not signed by the system's key.

All three checks are important. The first tells us whether the contents of a block have changed. The second makes sure that the blocks are still correctly connected. The third checks that the block really was signed by this system. Together, they provide the tamper-detection mechanism required by the assignment. When a check fails, the program reports which block failed and why.

While developing the program, I also found an issue with validation. Recalculating the hash directly on the original block could change the stored hash during the checking process. I solved this by making a copy of the block before recalculating its hash. In simple terms, the program now checks a copy instead of changing the original record. This means that validation only checks the blockchain and does not modify it.

## **Tamper Detection Demonstration**

The program includes a specific option (6, ADMIN only) to demonstrate what happens when an old record is changed. After at least one book has been borrowed, the program changes the title stored in Block \#1 to TAMPERED TITLE. It does not update the hash or create a new signature.

The program then validates the blockchain. Because the contents of the block no longer match its stored fingerprint, the validation fails and the program reports that tampering has been detected. This provides a simple demonstration of the main reason for using a blockchain in this project. The assignment also requires a demonstration that changing a past block breaks chain validation.

The change is only made in memory, and the program refuses to save a chain that fails validation. Therefore, restarting the program reloads the clean blockchain from data/chain.txt for another demonstration. You can also open data/chain.txt in a text editor, change a title, and start the program again. It will warn that the saved blockchain is INVALID.

## **Error Handling**

The program also tries to handle situations where an operation should not be allowed. It checks whether the registry files exist, whether they contain usable records (bad lines are reported and skipped), whether the key passphrase and librarian login are correct, whether book and member IDs are valid, whether a book is already borrowed, and whether a book is on loan to that member before allowing it to be returned. It also prevents new blocks from being added once the blockchain reaches its maximum size.

The program also handles invalid menu input. Input is read one line at a time, so overlong input cannot spill into the next prompt. If the user enters something that is not a number when a menu choice is expected, the program shows the menu again. If the input stream is closed, the program exits instead of continuing to ask for input. These checks help prevent the program from entering an unexpected state.

## **Current Limitations**

Although the system demonstrates the main ideas required for the blockchain part of the assignment, it also has some limitations. The blockchain is saved to data/chain.txt after every borrow or return and loaded again on start-up. The whole file is rewritten each time, which is fine for a small library but not for a large one.

There is a single signing key for the whole system, so the key proves that a block came from this program, and the signed librarian ID shows who made it. Separate keys for each librarian would be stronger. The PIN-based login protects the menu, but anyone who can edit data/librarians.txt can add an account. data/chain.txt and data/key.pem are listed in .gitignore and are not committed.

## **Conclusion**

The main purpose of this project was to apply the basic idea of blockchain to a library lending problem. Instead of treating a lending record as an ordinary piece of information that can simply be changed, the program creates a history of connected records. Each new record is linked to the previous one through its hash, lending actions are digitally signed, and only logged-in librarians can record them.

The process is therefore straightforward. The program first checks that the book and member exist. It then checks the book's previous lending activity. If the operation is valid, it creates a new block, signs it, calculates its hash, and adds it to the chain. Later, the program can examine the chain and determine whether any of its records have been changed.

The project therefore demonstrates the basic relationship between a library system and blockchain: **the registry tells the system what is valid, the blockchain keeps the history, cryptography helps protect the records, and validation checks whether the history has been altered.**

