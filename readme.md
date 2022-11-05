Tomas Ponce
02/24/22

wcclient.c
Sends GET requests to a server and caches the response.

To compile run make
To execute type ./wcclient and hit enter

Usage: Enter the name of a website that uses HTTP and a port number, if the server sends response code 200 - OK back, the page is cached in a file. The name of the file should be the date in which the request was made in YYYYMMDDhhmmss format. A list of cached webpages are stored in list.txt in <url> <filename> format. After five pages are cached, the least recent page is deleted and replaced with the new one.
