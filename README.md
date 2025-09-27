# Gigachad Server - A MultiThreaded Proof-Of-Concept HTTP Server in C++

## Features implemented
- GET and POST request methods
- Basic HTTP spec request parsing including parsing request line, headers and request body
- Host Header Validation
- Serving different file types via GET, for eg:-/index.html is served with `Content-Type: text/html` response header, different image formats are also served respectively
- Serving unidentified file types via Content-Type: octet-stream and Content-Disposition headers so that a download could be triggered on the client
- POST request with /upload route - A POST request containing valid JSON to `/upload` route will be uploaded to the server
- Error responses for bad requests, internal server errors, forbidden, not found.
- Multithreading, each connection is handled by the thread pool concurrently
- The server address, port and max number of threads in the thread pool can be specified via command line arguments
- Server logging and debug logging

## Usage
`./server`<br>
OR<br>
`./server <PORT> <IP_ADDRESS> <MAX_THREADS>`

- The SERVER_ROOT is set to the relative path `./res` from where you ran the server binary
- For the POST request to work, an `uploads` directory must exist inside the `res` folder

## Installation
Make sure `cmake` and `make` are installed on your system

- Clone the repository
- `cd HTTPServerFromScratch-C`
- `mkdir build`
- `cd build`
- `cmake ..`
- `make`
- `mkdir -p res/uploads`
- `./server` or `./server <PORT> <IP_ADDRESS> <MAX_THREADS>`

## Screenshots
<img width="1920" height="1003" alt="http_server_sc" src="https://github.com/user-attachments/assets/501d066f-cfe2-4374-a184-74d929419dee" />
