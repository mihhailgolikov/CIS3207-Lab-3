# Misha Golikov
# CIS3207-Lab-3


  EXPLANATION:

The Networked Spellchecker is implemented using the pthread.h and socket.h built-in C libraries,
along with string.h and in.h libraries as well. The server is multithreaded, and automatically synced between
a number of (multiple) clients, which can send words to the server to be spellchecked.

The server tries to match the words to dictionary words, hereby spell-checking them, and responds to all of the clients
in sync and concurrently. The server then outputs the responses to the log file, letting all of the clients
be able to see if any and all of the sent word(s) were spelled correctly or not (OK/WRONG).

  HOW-TO:

1. Access the project directory and call "make" in order to compile the server using the Makefile.
2. Call the server to run it, using any of the following arguments, if needed.

    a. "./server" - Run using default dictionary.txt file, and using default port number.
    
    b. "./server port" OR "./server customDictionary.txt" - Run using a custom port number or a custom dictionary file.
    
    c. "./server port customDictionary.txt" OR "./server customDictionary.txt port" - Run using both a custom port number
        and a custom dictionary file.
3. After running the server, sign into another terminal for the client to connect to the server using the call
"IPAddress port", where the port number is the one used in running the server (whether custom or default), and where the
I.P. Address is the address of the local machine "nc 127.0.0.1". Thus, using the default port, the second terminal 
will connect to the terminal by typing in "nc 127.0.0.1 7000".
4. Either of the users can now enter words into the terminal, to be processed by the server, spell checked, and output
into the logfile. More users can also connect to the server.
5. If one wants to exit the server, they can press the Escape key, and then the Enter key, as instructed by the server.
6. If more users want to connect to the server, they can simply input the nc command with the previously given parameters,
in order to connect.
7. Any of the users can now see the spell-checked output log file at real-time, as they type the words in, and concurrently
as well as synced with one anothers' words.



