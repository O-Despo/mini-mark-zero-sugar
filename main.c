/* C markdownish parser 
 * That has no allocated buffer
 * Written by Oliver D'Esposiot (O-Despo)
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>

typedef enum {P, HTML, HR, HEADER, UL_LIST, OR_LIST, B_CODE} Block;
typedef enum {L_NONE, EMPHASIS, BOLD, I_CODE, LINK} In_Line;
typedef enum {START, MIDDLE, END} Line_Pos;

typedef struct {
    Block block;
    In_Line in_line;
    int line;
    int col;
    int last_col;
    int level; /* Used for open close tags */
} State;
     /* Returns 1 if formating was done and no other block formattng
     * should be run*/

int m_fgetc(State *state, FILE *in_file, int c){
    c = fgetc(in_file);

    if(c == '\n'){
        state->line++;
        state->last_col = state->col;
        state->col = 0;
    } else if (c != EOF){
        state->col++;
    }

    return c;
}

int m_ungetc(State *state, FILE *in_file, int c){
    ungetc(c, in_file);

    if(c == '\n'){
        state->line--;
        state->col = state->last_col;
    } else if (c != EOF){
        state->col--;
    }

    return c;
}

short format_break(State *state, FILE *in_file, char c){
    /* short format_break(State *state, FILE *in_file, char c)
     *
     */
    short formated = 0;

    if(c == '='){
        fputs("</br>\n", stdout);
        formated = 1;
    }

    return formated;
}

short format_html(State *state, FILE *in_file, char c){
    /* int format_html(State *state, FILE *in_file, char c){
     * If a html elm is matched than formatting is stoped until its close
     */
    short formated = 0;

    if(c == '<'){
        state->level = 1;
        formated = 1;

        fputc(c, stdout);
        c = m_fgetc(state, in_file, c);
        if(c == '<'){ /* Special espcae for single tags*/
               state->level = 0;

                while(c != '>' && (c = m_fgetc(state, in_file, c)) != EOF){ 
                    fputc(c, stdout); 
                }
        } else {
            /* If its multi tag then move through html until there are as many closing
             * as opening tags */
            fputc(c, stdout);

            while(state->level != 0 && (c = m_fgetc(state, in_file, c)) != EOF) {
                
                if (c == '<') {
                    fputc(c, stdout);
                    c = m_fgetc(state, in_file, c); /* TODO check for EOF */
                    
                    if (c == '/') {
                        state->level--;
                        fputc(c, stdout);
                    } else if (c != '<'){
                        state->level++;
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

int format_code(State *state, FILE *in_file, char c){
    int formated = 0;
    if(c == '`'){
        state->level = 1;
        while((c = m_fgetc(state, in_file, c)) != EOF && c == '`'){
            
            state->level++;
        }

        if(state->level != 3){ /* if not 3 repalce ' and leave */
            for(;state->level > 0; state->level--){
                fputc('`', stdout);  
            }
            fputc(c, stdout);

        } else {
            state->block = B_CODE;
            formated = 1;
            state->level = 0;

            fputs("<pre><code>", stdout);
            fputc(c, stdout);

            while(state->level != 3 && (c = m_fgetc(state, in_file, c)) != EOF){
                
                if(c == '`'){
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

int format_header(State *state, FILE *in_file, char c){
    /* short format_header(State *state, FILE *in_file, char c){
     *
     * Formats headers modifyes state. Whole line operator.
     * Uses level to track header level up to 6.
     *

     */
    int formated = 0;
    if(c == '#'){
        state->level = 1;

        while((c = m_fgetc(state, in_file, c)) == '#'){
            
            state->level++;
        }

        /* Max header level is 6 */
        if(state->level >= 7){
            state->level = 6;
        }

        state->block = HEADER;
        fprintf(stdout, "<h%d>", state->level);
        formated = 1;
    }

    return formated;
}

int format_hr(State *state, FILE *in_file, char c){
    int formated = 0;
    if(c == '*'){
        /* NOTE: I kinda like this structure by checking a char and then ungeting if not
         * I can pretty much use the c in the main function as a 1 charater buffer */
        c = m_fgetc(state, in_file, c); /* TODO: Acount for EOF */
        

        if (c == '*') {
            fputs("<hr />", stdout); 

            while ((c = m_fgetc(state, in_file, c)) != EOF && c != '\n') {
                
            };

            if (c == '\n') {
                m_ungetc(state, in_file, c);
            }
            formated = 1;
            state->block = HR;
        } else {
            m_ungetc(state, in_file, c); /* TODO: Acount for error */
        }
    }

    return formated;
}

int format_ul_list(State *state, FILE *in_file, char c){
    int formated = 0;
    if (c == '*' || c == '+' || c == '-') {
        c = m_fgetc(state, in_file, c);
        

        if (c == ' ') {
            formated = 1;

            if (state->block != UL_LIST){
                fputs("<ul>\n", stdout);
            }
            state->block = UL_LIST;

            fputs("<li>", stdout);
        } else if (state->block == UL_LIST) { /* if in UL but not matched close list */
            state->block = P;
            fputs("</ul>\n", stdout);

            m_ungetc(state, in_file, c);
        } else {
            m_ungetc(state, in_file, c);
        }
    } else if (state->block == UL_LIST){ /* if in UL but not matched close list */
        state->block = P;
        fputs("</ul>\n", stdout);
    }

    return formated;
}

int format_ol_list(State *state, FILE *in_file, char c){
    int formated = 0;
    if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' ||
            c == '5' || c == '6' || c == '7' || c == '8' || c == '9') {
        c = m_fgetc(state, in_file, c);
        

        if (c == '.') {
            formated = 1;

            if (state->block != OR_LIST){
                fputs("<ol>\n", stdout);
            }
            state->block = OR_LIST;

            fputs("<li>", stdout);
        } else if (state->block == OR_LIST) { /* if in UL but not matched close list */
            state->block = P;
            fputs("</ol>\n", stdout);

            m_ungetc(state, in_file, c);
        } else {
            m_ungetc(state, in_file, c);
        }
    } else if (state->block == OR_LIST){ /* if in UL but not matched close list */
        state->block = P;
        fputs("</ol>\n", stdout);
    }

    return formated;
}

short in_line_emphasis(State *state, FILE *in_file, char c){ /* MUST COME AFTER BOLD */
    short formated = 0;

    if (c == '*' || c == '_'){
        formated = 1;           
        c = m_fgetc(state, in_file, c);
        

        if(c == '*' || c == '_'){
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

short in_line_code(State *state, FILE *in_file, char c){ /* MUST COME AFTER BOLD */
    short formated = 0;

    if (c == '`'){
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

short escape(State *state, FILE *in_file, char c){
    short escaped = 0;

    if (c == '\\') {
        escaped = 1;
        c = m_fgetc(state, in_file, c);
        fputc(c, in_file);
    }

    return escaped;
}

short format_line_break(State *state, FILE *in_file, char c){
    short escaped = 0;

    if (c == '\\') {
        escaped = 1;
        c = m_fgetc(state, in_file, c);
        
        fputc(c, in_file);
    }

    return escaped;
}

short in_line_link(State *state, FILE *in_file, char c){
    short formated = 0;

    if (c == '[') {
        fputs("<a href=\"", stdout);
        while ((c = m_fgetc(state, in_file, c)) != EOF && c != ']') {
            
            fputc(c, stdout);
        }

        if (c == EOF){ /* TODO: do this everywhere */
            printf("ERR: EOF before link close\n");
        }

        fputs("\">", stdout);
        c = m_fgetc(state, in_file, c);
        
        if(c == '('){
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

short close_blocks(State *state){
    short closed = 0;

    if(state->block == P){
        fputs("</p>", stdout);
        closed = 1;
    } else if (state->block == HEADER){
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


short close_in_lines(State *state){
    short closed = 0;

    if(state->in_line == L_NONE){
        closed = 1; 
    } else if (state->in_line == EMPHASIS){
        fputs("</em>", stdout);  
    } else if (state->in_line == BOLD) {
        fputs("</strong>", stdout);  
    } else if (state->in_line == LINK){
        fputs("</a>", stdout);
    } else if (state->in_line == I_CODE){
        fputs("</code></pre>", stdout);  
    }

    state->in_line = L_NONE; /* in lines do not conute after end of line */
    return closed;
}

int main(int argc, char *argv[]){
    int c;
    FILE *in_file;
    State state = {P, L_NONE, 1, 1, 1, 0}; /* Set the frist line to line start */

    /* Check for correct number of argumnets */
    if(argc != 2){
        errx(EXIT_FAILURE, "Incorrect number of arguments got %d expexted 1\n", argc - 1);
    }

    in_file = fopen(argv[1], "r");

    /* Check that file opened properly */
    if(in_file == NULL){
        err(EXIT_FAILURE, "Error opening in file %s\n", argv[2]);
    }

    /* Start processing file */
    while(EOF != (c = m_fgetc(&state, in_file, c))){

        if (c == '\n') {
            close_blocks(&state);
            close_in_lines(&state);

            fputc(c, stdout);

        } else if (state.col == 1) {
            if (!(
                format_break(&state, in_file, c)   ||
                format_ol_list(&state, in_file, c) ||
                format_ul_list(&state, in_file, c) ||
                format_html(&state, in_file, c)    ||
                format_code(&state, in_file, c)    ||
                format_header(&state, in_file, c)  || 
                format_hr(&state, in_file, c))) {

                state.block = P;
                fputs("<p>", stdout);

                if (!(
                    in_line_emphasis(&state, in_file, c) ||
                    in_line_code(&state, in_file, c)     ||
                    in_line_link(&state, in_file, c))) {

                    fputc(c, stdout);
                }
            }
        } else {
            if (!(
                in_line_emphasis(&state, in_file, c) ||
                in_line_code(&state, in_file, c)     ||
                in_line_link(&state, in_file, c))) {
                fputc(c, stdout);
            }
        }
    }

    fclose(in_file);

    return 0;
}
