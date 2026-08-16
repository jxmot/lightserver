#!/usr/bin/env node
/*
    espmin - A NodeJS(v22.23.2) utility that takes formatted HTML (including CSS, JS) and 
    minimizes it. It will remove:
      * comments
      * indentation
      * end-of-line characters

    And then it will create a CPP file. The file name comes from the input file's 
    name and extension. For example, `index.html` will result in a file named `index_html.cpp`.
    If the file already exists it will be over written.

    Inside of the CPP file:
      * the first line is `const char NEW_NAME[] PROGMEM = R"rawliteral(`, where using the 
      example above `NEW_NAME` will be `index_html`
      * the next line is the minimized file contents
      * the last line is `)rawliteral";`, followed by an empty line

*/
const fs = require('fs');
const path = require('path');

if (process.argv.length < 3) {
  console.error('Usage: node espmin.js <filename>');
  process.exit(1);
}

// Read input file
let input;
try {
  input = fs.readFileSync(process.argv[2], 'utf8');
} catch (e) {
  console.error('Error reading file:', e.message);
  process.exit(1);
}

