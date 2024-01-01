/* mini-mark-zero-sugar
 *
 * mimi-mark-zero-sugar is a zero-sugar-markdown parser. zero-sugar beacuse it
 * both dose not use a buffer directly in a program, it only uses getc and
 * ungetc (ungetc is only ever called once beacuse by the c standard thats all
 * you are garenteed). I did this as more of a challenge then to makke
 * anything practical. So don't try and use this for anything practical for
 * that see cmark(https://github.com/commonmark/cmark). This dose not
 * impiment common mark or even the standard from original markdown. I made
 * my own zero-sugar markdown like syntax beacuse dealing with certian things
 * in markdown (like link references) would have sucked. Once again this was
 * more of a what if rather than a use this, don't use this for anything
 * important.
 *
 * Written by Oliver D'Esposiot (O-Despo)
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>

#define CHECK_BLOCKS    (format_break(&state, in_file, c)   ||\
                        format_ol_list(&state, in_file, c) ||\
                        format_ul_list(&state, in_file, c) ||\
                        format_html(&state, in_file, c)    ||\
                        format_code(&state, in_file, c)    ||\
                        format_header(&state, in_file, c)  ||\
                        format_hr(&state, in_file, c))

#define CHECK_INLINES   (in_line_emphasis_bold(&state, in_file, c) ||\
                        in_line_code(&state, in_file, c)     ||\
                        in_line_link(&state, in_file, c))

typedef enum {B_NONE, P, HTML, HR, HEADER, UL_LIST, OR_LIST, B_CODE} Block;
typedef enum {L_NONE, EMPHASIS, BOLD, I_CODE, LINK} In_Line;
typedef enum {START, MIDDLE, END} Line_Pos;

typedef struct {
    Block block;
    In_Line in_line;
    int line;
    int col;
    int last_col;
    int level;
} State;

int m_fgetc(State *state, FILE *in_file, int c) {
    /* int m_fgetc(State *state, FILE *in_file, int c)
     *
     * m_fgetc wraps getc and keeps track of cols and lines.
     * Returns the new charater. Side effect cols and lines in state.
     */
    c = fgetc(in_file);

    if(c == '\n') {
        state->line++;
        state->last_col = state->col;
        state->col = 0;
    } else if (c != EOF) {
        state->col++;
    }

    return c;
}

int m_ungetc(State *state, FILE *in_file, int c) {
    /* int m_ungetc(State *state, FILE *in_file, int c)
     *
     * m_ungetc wraps ungetc and keeps track of cols and lines.
     * Returns the charater passed. Side effect cols and lines in state.
     */
    ungetc(c, in_file);

    if (c == '\n') {
        state->line--;
        state->col = state->last_col;
    } else if (c != EOF) {
        state->col--;
    }

    return c;
}

short format_break(State *state, FILE *in_file, char c) {
    /* short format_break(State *state, FILE *in_file, char c)
     *
     * Will format a line break. Follows block format behavoir.
     */
    short formated = 0;

    if (c == '=') {
        fputs("</br>", stdout);
        formated = 1;
    }

    return formated;
}

short format_html(State *state, FILE *in_file, char c) {
    /* short format_html(State *state, FILE *in_file, char c){
     *
     * Formates a html block. This is a consuming block it will move thorugh
     * the file until a closing charater is found. Uses level to keep track of
     * open and closed html tags. Follows block format behavoir.
     */
    short level = 1;
    short formated = 0;

    if(c == '<') {
        formated = 1;
        state->block = HTML;

        fputc(c, stdout);
        c = m_fgetc(state, in_file, c);

        if (c == '<') {
            /* Special espcae for single tags*/
            level = 0;

            while (c != '>' && (c = m_fgetc(state, in_file, c)) != EOF) {
                fputc(c, stdout);
            }
        } else {
            /* If its multi tag then move through html until there are as many
             * closing as opening tags */
            fputc(c, stdout);

            while (level != 0 && (c = m_fgetc(state, in_file, c)) != EOF) {

                if (c == '<') {
                    fputc(c, stdout);
                    c = m_fgetc(state, in_file, c);

                    if (c == '/') {
                        level--;
                        fputc(c, stdout);
                    } else if (c != '<') {
                        level++;
                        fputc(c, stdout);
                    }
                } else {
                    fputc(c, stdout);
                }
            }
        }
    }

    return formated;
}

int format_code(State *state, FILE *in_file, char c) {
    /* int format_code(State *state, FILE *in_file, char c)
     *
     * Formates a code block. This is a consuming block it will move thorugh
     * the file until a closing charater is found. Uses level to keep track of
     * open and closed html tags. Follows block format behavoir.
     */
    int formated = 0;
    if (c == '`') {
        c = m_fgetc(state, in_file, c);

        if (c == '`') {
            state->block = B_CODE;
            formated = 1;
            fputs("<pre><code>", stdout);

            state->level = 0;

            while (state->level != 2 &&
                    (c = m_fgetc(state, in_file, c)) != EOF) {
                if (c == '`') {
                    state->level++;
                } else {
                    state->level = 0;
                    fputc(c, stdout);
                }
            }

            fputs("</pre></code>", stdout);
        }
    }

    return formated;
}

short format_header(State *state, FILE *in_file, char c) {
    /* short format_header(State *state, FILE *in_file, char c){
     *
     * Formats headers modifyes state. Whole line operator.
     * Uses level to track header level up to 6. Follow block format behavoir.
     */
    int formated = 0;
    if (c == '#') {
        state->level = 1;
        formated = 1;

        while ((c = m_fgetc(state, in_file, c)) == '#') {
            state->level++;
        }

        /* Max header level is 6 */
        if (state->level >= 7) {
            state->level = 6;
        }

        state->block = HEADER;
        fprintf(stdout, "<h%d>", state->level);
    }

    return formated;
}

short format_hr(State *state, FILE *in_file, char c) {
    /* short format_hr(State *state, FILE *in_file, char c) {
     *
     * Format horozontal rules. If anything comes after a horizontal rule
     * it is removed. Follows block format behavoir.
     */
    int formated = 0;

    if (c == '-') {
        c = m_fgetc(state, in_file, c);

        if (c == '-') {
            fputs("<hr/>", stdout);

            while ((c = m_fgetc(state, in_file, c)) != EOF && c != '\n') {};

            if (c == '\n') {
                m_ungetc(state, in_file, c);
            }

            formated = 1;
            state->block = HR;
        } else {
            m_ungetc(state, in_file, c);
        }
    }

    return formated;
}

short format_ul_list(State *state, FILE *in_file, char c) {
    /* short format_ul_list(State *state, FILE *in_file, char c)
     *
     * Format a unorderd list. Dose not consume. Follows block format behavoir.
     */
    int formated = 0;
    if (c == '*' || c == '+' || c == '-') {
        c = m_fgetc(state, in_file, c);


        if (c == ' ') {
            formated = 1;

            if (state->block != UL_LIST) {
                fputs("<ul>\n", stdout);
            }
            state->block = UL_LIST;

            fputs("<li>", stdout);
        } else if (state->block == UL_LIST) {
            /* if in UL but not matched close list */
            state->block = B_NONE;
            fputs("</ul>\n", stdout);

            m_ungetc(state, in_file, c);
        } else {
            m_ungetc(state, in_file, c);
        }
    } else if (state->block == UL_LIST) {
        /* if in UL but not matched close list */
        state->block = B_NONE;
        fputs("</ul>\n", stdout);
    }

    return formated;
}

short format_ol_list(State *state, FILE *in_file, char c) {
    /* short format_ol_list(State *state, FILE *in_file, char c)
     *
     * Format ordered list. Dose not consume. Follows block format behavoir.
     */
    int formated = 0;
    if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
            c == '5' || c == '6' || c == '7' || c == '8' || c == '9') {
        c = m_fgetc(state, in_file, c);

        if (c == '.') {
            formated = 1;
            if (state->block != OR_LIST) {
                fputs("<ol>\n", stdout);
            }
            state->block = OR_LIST;

            fputs("<li>", stdout);
        } else if (state->block == OR_LIST) {
            /* if in OL but not matched close list */
            state->block = B_NONE;
            fputs("</ol>\n", stdout);

            m_ungetc(state, in_file, c);
        } else {
            m_ungetc(state, in_file, c);
        }
    } else if (state->block == OR_LIST) {
        /* if in OR but not matched close list */
        state->block = B_NONE;
        fputs("</ol>\n", stdout);
    }

    return formated;
}

short in_line_emphasis_bold(State *state, FILE *in_file, char c) {
    /* short in_line_emphasis_bold(State *state, FILE *in_file, char c)
    *
    * Will format a inline emphasis or bold. Follows inline format behavior.
    */
    short formated = 0;

    if (c == '*' || c == '_') {
        formated = 1;
        c = m_fgetc(state, in_file, c);


        if(c == '*' || c == '_') {
            if (state->in_line == BOLD) {
                fputs("</strong>", stdout);
                state->in_line = L_NONE;
            } else if (state->in_line == L_NONE) {
                fputs("<strong>", stdout);
                state->in_line = BOLD;
            }
        } else {
            m_ungetc(state, in_file, c);

            if (state->in_line == EMPHASIS) {
                fputs("</em>", stdout);
                state->in_line = L_NONE;
            } else if (state->in_line == L_NONE) {
                fputs("<em>", stdout);
                state->in_line = EMPHASIS;
            }
        }
    }

    return formated;
}

short in_line_code(State *state, FILE *in_file, char c) {
    /* short in_line_code(State *state, FILE *in_file, char c)
     *
     * Will format in line code. Follows inline format behavior.
     */
    short formated = 0;

    if (c == '`') {
        formated = 1;
        if(state->in_line == I_CODE) {
            fputs("</code>", stdout);
            state->in_line = L_NONE;
        } else if (state->in_line == L_NONE) {
            fputs("<code>", stdout);
            state->in_line = I_CODE;
        }
    }

    return formated;
}

short escape(State *state, FILE *in_file, char c) {
    /* short escape(State *state, FILE *in_file, char c)
     *
     * Return true the character has been escaped.
     */
    short escaped = 0;

    if (c == '\\') {
        escaped = 1;
        c = m_fgetc(state, in_file, c);
        fputc(c, in_file);
    }

    return escaped;
}

short in_line_link(State *state, FILE *in_file, char c) {
    /* short in_line_link(State *state, FILE *in_file, char c) {
     *
     * Consumes a link will error if its not closed.
     */
    short formated = 0;

    if (c == '[') {
        fputs("<a href=\"", stdout);

        while ((c = m_fgetc(state, in_file, c)) != EOF && c != ']') {
            fputc(c, stdout);
        }

        if (c == EOF) { /* TODO: do this everywhere */
            printf("ERR: EOF before link close\n");
        }

        fputs("\">", stdout);
        c = m_fgetc(state, in_file, c);

        if(c == '(') {
            while ((c = m_fgetc(state, in_file, c)) != EOF && c != ')') {
                fputc(c, stdout);
            }
            fputs("</a>", stdout);
        } else {
            printf("ERR: Linke with no '(' got '%c'\n", c);
            fputs("</a>", stdout);
        }
        formated = 1;
    }
    return formated;
}

short close_blocks(State *state) {
    /* short close_blocks(State *state)
     *
     * Will close open blocks.
     */
    short closed = 0;

    if(state->block == P) {
        fputs("</p>", stdout);
        closed = 1;
    } else if (state->block == HEADER) {
        fprintf(stdout, "</h%d>", state->level);
        closed = 1;
    } else if (state->block == HR) {
        closed = 1;
    } else if (state->block == UL_LIST) {
        fputs("</li>", stdout);
        closed = 1;
    }

    return closed;
}

short close_in_lines(State *state) {
    /* short close_in_lines(State *state) {
     *
     * Will close open in line elements.
     */
    short closed = 0;

    if(state->in_line == L_NONE) {
        closed = 1;
    } else if (state->in_line == EMPHASIS) {
        fputs("</em>", stdout);
    } else if (state->in_line == BOLD) {
        fputs("</strong>", stdout);
    } else if (state->in_line == LINK) {
        fputs("</a>", stdout);
    } else if (state->in_line == I_CODE) {
        fputs("</code></pre>", stdout);
    }

    state->in_line = L_NONE;
    return closed;
}

int main(int argc, char *argv[]) {
    int c;
    FILE *in_file;
    State state = {P, L_NONE, 1, 0, 1, 0};

    /* Check for correct number of arguments */
    if(argc != 2) {
        errx(EXIT_FAILURE, "Incorrect number of arguments got %d expected 1\n",
             argc - 1);
    }

    in_file = fopen(argv[1], "r");

    /* Check that file opened properly */
    if(in_file == NULL) {
        err(EXIT_FAILURE, "Error opening in file %s\n", argv[2]);
    }

    /* Start processing file */
    while(EOF != (c = m_fgetc(&state, in_file, c))) {

        if (c == '\n' && state.last_col != 0) {

            close_blocks(&state);
            close_in_lines(&state);
            fputc(c, stdout);

        } else if (state.col == 1) {

            if (!CHECK_BLOCKS) {

                state.block = P;
                fputs("<p>", stdout);

                if (!CHECK_INLINES) {
                    fputc(c, stdout);
                }
            }
        } else if (c != '\n') {

            if (!CHECK_INLINES) {
                fputc(c, stdout);
            }
        }
    }

    fclose(in_file);
    return 0;
}
