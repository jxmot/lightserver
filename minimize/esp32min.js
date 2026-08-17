#!/usr/bin/env node
/*
    espmin - A NodeJS(v22.23.2) utility that takes formatted HTML (including CSS, JS) and
    minimizes it. It will remove:
      * comments
      * indentation
      * end-of-line characters

    And then it will create a CPP file. The file name comes from the input file's
    name and extension. For example, `index.html` will result in a file named `_index_html.cpp`.
    If the file already exists it will be over written.

    Inside of the CPP file:
      * the first line is `const char NEW_NAME[] PROGMEM = R"rawliteral(`, where using the
      example above `NEW_NAME` will be `index_html`
      * the next line is the minimized file contents
      * the last line is `)rawliteral";`, followed by an empty line

    The generated CPP file and a minimized HTML test file are written to the
    current working directory. The minimized HTML test file has the input
    filename with an underscore prepended. For example, `index.html` will result
    in `_index.html`.
*/
const fs = require('fs');
const path = require('path');

if (process.argv.length !== 3) {
    console.error('Usage: node esp32min.js <filename>');
    process.exit(1);
}

const inputPath = process.argv[2];
const inputName = path.basename(inputPath);
const inputExtension = path.extname(inputName);

if (inputExtension.toLowerCase() !== '.html') {
    console.error('Error: input file must have an .html extension.');
    process.exit(1);
}

let input;
try {
    input = fs.readFileSync(inputPath, 'utf8');
} catch (e) {
    console.error('Error reading file:', e.message);
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
    let i = 0;
    let state = 'code';

    while (i < source.length) {
        const c = source[i];
        const next = source[i + 1];

        if (state === 'code') {
            if (c === '"' || c === "'") {
                result += c;
                state = c === '"' ? 'double' : 'single';
                i++;
                continue;
            }

            if (c === '`') {
                result += c;
                state = 'template';
                i++;
                continue;
            }

            if (c === '/' && next === '/') {
                i += 2;
                while (i < source.length && source[i] !== '\n' && source[i] !== '\r')
                    i++;
                continue;
            }

            if (c === '/' && next === '*') {
                i += 2;
                while (i < source.length && !(source[i] === '*' && source[i + 1] === '/'))
                    i++;
                if (i < source.length)
                    i += 2;
                continue;
            }

            result += c;
            i++;
            continue;
        }

        if (c === '\\') {
            result += c;
            if (i + 1 < source.length)
                result += source[i + 1];
            i += 2;
            continue;
        }

        result += c;

        if ((state === 'double' && c === '"') || (state === 'single' && c === "'") || (state === 'template' && c === '`'))
            state = 'code';

        i++;
    }

    return result;
}

function removeComments(source) {
    let result = removeHtmlComments(source);

    const stylePattern = /<style\b[^>]*>[\s\S]*?<\/style\s*>/gi;
    result = result.replace(stylePattern, match => {
        const openEnd = match.indexOf('>') + 1;
        const closeStart = match.toLowerCase().lastIndexOf('</style');
        const openTag = match.slice(0, openEnd);
        const css = match.slice(openEnd, closeStart);
        const closeTag = match.slice(closeStart);
        return openTag + removeCssComments(css) + closeTag;
    });

    const scriptPattern = /<script\b[^>]*>[\s\S]*?<\/script\s*>/gi;
    result = result.replace(scriptPattern, match => {
        const openEnd = match.indexOf('>') + 1;
        const closeStart = match.toLowerCase().lastIndexOf('</script');
        const openTag = match.slice(0, openEnd);
        const script = match.slice(openEnd, closeStart);
        const closeTag = match.slice(closeStart);
        return openTag + removeJsComments(script) + closeTag;
    });

    return result;
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
        const marker = `___ESP32MIN_PROTECTED_${index}___`;
        result = result.replace(marker, part);
    });

    return result;
}

const minimized = collapseWhitespace(removeComments(input));

if (minimized.includes(')rawliteral')) {
    console.error('Error: input contains the sequence ")rawliteral", which cannot be safely embedded in a raw C++ literal.');
    process.exit(1);
}

const baseName = path.basename(inputName, inputExtension);
const cppName = `_${baseName}_${inputExtension.slice(1)}.cpp`;
const cppVariable = `${baseName}_${inputExtension.slice(1)}`;
const minimizedHtmlName = `_${inputName}`;

const cppContent = `const char ${cppVariable}[] PROGMEM = R"rawliteral(\n${minimized}\n)rawliteral";\n`;
const outputHtmlPath = path.join(process.cwd(), minimizedHtmlName);
const outputCppPath = path.join(process.cwd(), cppName);

try {
    fs.writeFileSync(outputHtmlPath, minimized, 'utf8');
    fs.writeFileSync(outputCppPath, cppContent, 'utf8');
} catch (e) {
    console.error('Error writing output file:', e.message);
    process.exit(1);
}

console.log(`Created ${minimizedHtmlName}`);
console.log(`Created ${cppName}`);
