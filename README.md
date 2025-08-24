# C Web Server

A simple web server built from scratch in **C** as part of my hobby experiments with networking, HTTP, and low-level socket programming.  

## Features
- Handles basic HTTP requests  
- Serves static HTML pages (and CSS)  
- Lightweight and minimal design  

## Getting Started
1. Clone the repository:
   ```bash
   git clone https://github.com/<your-username>/<your-repo-name>.git
   cd <your-repo-name>
   ```
2. Compile the server:
   ```bash
   gcc server.c -o server
   ```
3. Run the server:
   ```bash
   ./server
   ```
4. Open in your browser:
   ```bash
   http://localhost:8080
   ```

## Example
You can serve an index.html file and style it with CSS. Place them in the same directory as the server executable.

## Notes
- This project is purely for learning and hobby purposes
- Future improvements may include support for multiple clients, more HTTP features, and dynamic content
