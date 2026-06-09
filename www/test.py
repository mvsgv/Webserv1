#!/usr/bin/env python3
import os

print("Content-Type: text/html")
print("")
print("<h1>CGI Test - Hello from Webserv</h1>")
print("<h2>Request Information</h2>")
print("<ul>")
print("  <li>METHOD: " + os.environ.get('REQUEST_METHOD', 'GET') + "</li>")
print("  <li>URI: " + os.environ.get('REQUEST_URI', '/') + "</li>")
print("  <li>SERVER: " + os.environ.get('SERVER_SOFTWARE', 'unknown') + "</li>")
print("</ul>")
print("<p>CGI is working!</p>")
