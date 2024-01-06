# mini-mark-zero-sugar

mimi-mark-zero-sugar is a zero-sugar-markdown parser. zero-sugar because it both dose not use a buffer directly in a program, it only uses getc and ungetc (ungetc is only ever called once because by the c standard that's all you are guaranteed). I did this as more of a challenge then to make anything practical. It would be far more practical to use a buffer. So don't try and use this for anything practical for that see [cmark](https://github.com/commonmark/cmark). 

## Usage 

`./mmzs.o infile.txt outfile.html`: process the infile and output it to the given file

`./mmzs.o infile.txt`: prints to stdout

## mini-mark-zero-sugar standard

This dose not implement common mark or even the standard from original markdown. I made my own zero-sugar markdown like syntax because dealing with certain things in markdown (like link references) would have sucked. Once again this was more of a what if rather than a use this, don't use this for anything important.

There are two types of formatting in mini-mark-zero-sugar **blocks** and **inline**. 

**Notes on this implementation**. The behavior of inline formatting when nested is undefined. Inline formatting will be closed at the end of a line. Single line block formatting will be closed will be closed at the end of the line. Multi-line block formatting will be closed when that formatting is not on the next line.
### Blocks

Blocks must start at the beginning of the line.

- `#` denotes a header `<h1>` 
	- the level of the header is determined by the number of `#`'s in a row. `####` would produce `<h4>`
	- The max level is 6 after that `#` are removed from the output but not counted to the total
- `\n` (A empty line) will become a `<br>`
- `- ` ,  `* ` and `+ ` denote a unordered list
	- The can be used together in the same list
- \`\` (2 back ticks)  denotes the start of a code block
	- Code blocks must be closed with \`\`
	- No additional formatting is done in code blocks 
- `<` Will start a HTML block
	- Mini-mark dose not check if the contents of HTML blocks are valid html
	- In HTML blocks if you use a HTML tag that is only a single tag (`<hr>` or `<\hr>`) you must use  two opening `<<`(`<<hr>`) this includes if its the first tag
	- No additional formatting is done in code blocks 
- `=` denotes a line break `<br>`
- `--` denotes a horizontal rule `<hr>`
### Inline formatting 

Inline formatting cannot be nested. No additions formatting or checks are done inside of inline formatting. Inline formatting is closed at the end of a line.

- `*word*` and `_word_`  will create a emphasis block `<em>word</em>`
	- note that if you use `* ` at the start of a line it will be counted as a unordered list
- `**word**`, `__word__`, `_*word*_`, `*_word_*` will create a bold block `<bold>word</bold>`
- ``` `code` ``` will make a inline code block `<code>code</code>`
- `[address](word to be linked)` will create a link `<a href="address">word to be linked</a>`
### Character Escaping

Characters can be escaped with a `\` not matter what character comes after `\` it will be print at that character. This will also block all inline and block formatting. However a `\` will not escape in non formatted blocks those being HTML and Code blocks.  

## Copyright

Written by O-Despo (Oliver D'Esposito)
Although I did not directly use the source code for markdown in this project I did take inspiration. So the copyright follows.
Copyright © 2004, John Gruber
http://daringfireball.net/
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

Neither the name “Markdown” nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

This software is provided by the copyright holders and contributors “as is” and any express or implied warranties, including, but not limited to, the implied warranties of merchantability and fitness for a particular purpose are disclaimed. In no event shall the copyright owner or contributors be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including, but not limited to, procurement of substitute goods or services; loss of use, data, or profits; or business interruption) however caused and on any theory of liability, whether in contract, strict liability, or tort (including negligence or otherwise) arising in any way out of the use of this software, even if advised of the possibility of such damage.

