#!/usr/bin/env node
/*
    esp32min - A NodeJS(v22.23.2) utility that takes formatted HTML (including CSS and JS)
    and minimizes it. It removes comments, unnecessary whitespace, and whitespace between
    HTML tags.

    The utility creates two files in the current working directory:
      * _INPUT.html - the minimized HTML
      * _INPUT_html.cpp - the minimized HTML inside a PROGMEM raw C++ string

    For example, `index.html` produces `_index.html` and `_index_html.cpp`.
    If either output file already exists, it is overwritten.

    The generated C++ variable name is based on the input filename. For example,
    `index.html` produces `index_html`.
*/
const fs = require('fs');
const path = require('path');

const usage = 'Usage: node esp32min.js <html-file>';

if (process.argv.length !== 3) {
    console.error(usage);
    process.exit(1);
}

const inputPath = process.argv[2];
const inputName = path.basename(inputPath);
const extension = path.extname(inputName);

if (extension.toLowerCase() !== '.html') {
    console.error('Error: input file must have an .html extension.');
    process.exit(1);
}

let input;
try {
    input = fs.readFileSync(inputPath, 'utf8');
} catch (error) {
    console.error(`Error reading "${inputPath}": ${error.message}`);
    process.exit(1);
}

function removeHtmlComments(source) {
    return source.replace(/<!--[\s\S]*?-->/g, '');
}

function removeCssComments(source) {
    return source.replace(/\/\*[\s\S]*?\*\//g, '');
}

function removeJsComments(source) {
    let result = '';
    let index = 0;
    let state = 'code';

    while (index < source.length) {
        const current = source[index];
        const next = source[index + 1];

        if (state === 'code') {
            if (current === '"' || current === "'") {
                result += current;
                state = current === '"' ? 'double' : 'single';
                index++;
                continue;
            }

            if (current === '`') {
                result += current;
                state = 'template';
                index++;
                continue;
            }

            if (current === '/' && next === '/') {
                index += 2;
                while (index < source.length && source[index] !== '\n' && source[index] !== '\r')
                    index++;
                continue;
            }

            if (current === '/' && next === '*') {
                index += 2;
                while (index < source.length && !(source[index] === '*' && source[index + 1] === '/'))
                    index++;
                if (index < source.length)
                    index += 2;
                continue;
            }

            result += current;
            index++;
            continue;
        }

        if (current === '\\') {
            result += current;
            if (index + 1 < source.length)
                result += source[index + 1];
            index += 2;
            continue;
        }

        result += current;

        if ((state === 'double' && current === '"') || (state === 'single' && current === "'") || (state === 'template' && current === '`'))
            state = 'code';

        index++;
    }

    return result;
}

function removeComments(source) {
    let result = removeHtmlComments(source);
    result = result.replace(/<style\b[^>]*>[\s\S]*?<\/style\s*>/gi, match => {
        const contentStart = match.indexOf('>') + 1;
        const contentEnd = match.toLowerCase().lastIndexOf('</style');
        return match.slice(0, contentStart) + removeCssComments(match.slice(contentStart, contentEnd)) + match.slice(contentEnd);
    });

    return result.replace(/<script\b[^>]*>[\s\S]*?<\/script\s*>/gi, match => {
        const contentStart = match.indexOf('>') + 1;
        const contentEnd = match.toLowerCase().lastIndexOf('</script');
        return match.slice(0, contentStart) + removeJsComments(match.slice(contentStart, contentEnd)) + match.slice(contentEnd);
    });
}

function collapseWhitespace(source) {
    const protectedParts = [];
    const protectedPattern = /<(pre|textarea)\b[^>]*>[\s\S]*?<\/\1\s*>/gi;

    let result = source.replace(protectedPattern, match => {
        const marker = `___ESP32MIN_PROTECTED_${protectedParts.length}___`;
        protectedParts.push(match);
        return marker;
    });

    result = result.replace(/\s+/g, ' ').replace(/>\s+</g, '><').trim();

    protectedParts.forEach((part, index) => {
        result = result.replace(`___ESP32MIN_PROTECTED_${index}___`, part);
    });

    return result;
}

const minimized = collapseWhitespace(removeComments(input));

if (minimized.includes(')rawliteral')) {
    console.error('Error: input contains ")rawliteral", which cannot be safely embedded in a C++ raw string literal.');
    process.exit(1);
}

const baseName = path.basename(inputName, extension);
const extensionName = extension.slice(1);
const cppName = `_${baseName}_${extensionName}.cpp`;
const cppVariable = `${baseName}_${extensionName}`;
const htmlName = `_${inputName}`;
const outputDirectory = process.cwd();

const cppContent = `const char ${cppVariable}[] PROGMEM = R"rawliteral(\n${minimized}\n)rawliteral";\n`;

try {
    fs.writeFileSync(path.join(outputDirectory, htmlName), minimized, 'utf8');
    fs.writeFileSync(path.join(outputDirectory, cppName), cppContent, 'utf8');
} catch (error) {
    console.error(`Error writing output files: ${error.message}`);
    process.exit(1);
}

console.log(`Created ${htmlName}`);
console.log(`Created ${cppName}`);
