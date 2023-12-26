/* C markdown parser
* Written by Oliver D'Esposiot (O-Despo)
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <err.h>

typedef enum {B_NONE, HTML, HR, HEADER, UL_LIST, OR_LIST, B_CODE, P} Block;
typedef enum {L_NONE, EMPHASIS, ODE} In_Line;
typedef enum {START, MIDDLE, END} Line_Pos;

typedef struct {
    Block block;
    In_Line in_line;
    Line_Pos line_pos;
    Line_Pos last_pos;
    int level; /* Used for open close tags */
} State;

void set_line_pos(State *state, char c);

int format_break(State *state, FILE *in_file, char c){
    /* short format_break(State *state, FILE *in_file, char c)
     *
     * Two END charecters in a row then insert a blank line. Write to file.
     * On format reuturns 1. Dose not effect state.
     */
    int formated = 0;

    if(state->line_pos == END && state->last_pos == END){
        fputs("</br>\n", stdout);
        formated = 1;
    }

    return formated;
}

int format_html(State *state, FILE *in_file, char c){
    /* int format_html(State *state, FILE *in_file, char c){
     * If a html elm is matched than formatting is stoped until its close
     *
     * NOTE: This implementation will have a bug when the html has single line html elments 
     * in it. It will set the level to low and make is so the html block is exited to soon
     */
    int formated = 0;

    if(c == '<'){
        state->level = 1;
        formated = 1;

        /* Check if this is a single tag */
        fputc(c, stdout);
        c = fgetc(in_file); /* TODO add EOF check */

        if(c == '/'){ /* if a singel tag loop through until close */
            state->level = 0;

            fputc(c, stdout);

            while(c != '>' && (c = fgetc(in_file)) != EOF){ 
                fputc(c, stdout); 
            }
        } else {
            /* If its multi tag then move through html until there are as many closing
             * as opening tags */
            fputc(c, stdout);

            while(state->level != 0 && (c = fgetc(in_file)) != EOF) {
                set_line_pos(state, c);
                if (c == '<') {
                    fputc(c, stdout);
                    c = fgetc(in_file); /* TODO check for EOF */

                    if (c == '/') {
                        state->level--;
                    } else {
                        state->level++;
                    }
                    fputc(c, stdout);
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
        while((c = fgetc(in_file)) != EOF && c == '`'){
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

            while(state->level != 3 && (c = fgetc(in_file)) != EOF){
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
     * Returns 1 if formating was done and no other block formattng
     * should be run
     */
    int formated = 0;
    if(c == '#'){
        state->level = 1;
        
        while((c = fgetc(in_file)) == '#'){
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
        c = fgetc(in_file); /* TODO: Acount for EOF */
    
        if (c == '*') {
            fputs("<hr />", stdout); 
            
            while ((c = fgetc(in_file)) != EOF && c != '\n') {};

            if (c == '\n') {
                ungetc(c, in_file);
            }
            formated = 1;
            state->block = HR;
        } else {
            ungetc(c, in_file); /* TODO: Acount for error */
        }
    }

    return formated;
}

int format_ul_list(State *state, FILE *in_file, char c){
    int formated = 0;
    if (c == '*' || c == '+' || c == '-') {
        c = fgetc(in_file);

        if (c == ' ') {
            formated = 1;

            if (state->block != UL_LIST){
                fputs("<ul>\n", stdout);
            }
            state->block = UL_LIST;

            fputs("<li>", stdout);
        } else if (state->block == UL_LIST) { /* if in UL but not matched close list */
            state->block = B_NONE;
            fputs("</ul>\n", stdout);
            ungetc(c, in_file);
        } else {
            ungetc(c, in_file);
        }
    } else if (state->block == UL_LIST){ /* if in UL but not matched close list */
        state->block = B_NONE;
        fputs("</ul>\n", stdout);
    }

    return formated;
}

int format_ol_list(State *state, FILE *in_file, char c){
    int formated = 0;
    if (c == '0' ||
        c == '1' || 
        c == '2' || 
        c == '3' ||
        c == '4' ||
        c == '5' ||
        c == '6' ||
        c == '7' || 
        c == '8' ||
        c == '9') {
        c = fgetc(in_file);

        if (c == '.') {
            formated = 1;

            if (state->block != OR_LIST){
                fputs("<ol>\n", stdout);
            }
            state->block = OR_LIST;

            fputs("<li>", stdout);
        } else if (state->block == OR_LIST) { /* if in UL but not matched close list */
            state->block = B_NONE;
            fputs("</ol>\n", stdout);
            ungetc(c, in_file);
        } else {
            ungetc(c, in_file);
        }
    } else if (state->block == OR_LIST){ /* if in UL but not matched close list */
        state->block = B_NONE;
        fputs("</ol>\n", stdout);
    }

    return formated;
}
void set_line_pos(State *state, char c){
    /* short set_line_pos(State *state, char c)
     *
     * Sets the current line position and the last pos. Mods state.
     * Dose not write to output. Dose not return value.
     * If `\n` the set END. If END set START. If START and not `\n` set MIDDLE.
     * When POS is set save last POS.
     */
    if(c == '\n'){
        state->last_pos = state->line_pos;
        state->line_pos = END;    
    } else if (state->line_pos == END) {
        state->last_pos = state->line_pos;
        state->line_pos = START;
    } else if (state->line_pos == START) {
        state->last_pos = state->line_pos;
        state->line_pos = MIDDLE; 
    }
}


short close_blocks(State *state){
    short closed = 0;
    if(state->block == B_NONE){
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

int main(int argc, char *argv[]){
    int c;
    FILE *in_file;
    State state = {B_NONE, L_NONE, END, END, 0}; /* Set the frist line to line start */
   
    /* Check for correct number of argumnets */
    if(argc != 2){
        errx(EXIT_FAILURE, "Incorrect number of arguments got %d expexted 1\n", argc - 1);
    }
    
    in_file = fopen(argv[1], "r");

    printf("%s\n", argv[1]);

    /* Check that file opened properly */
    if(in_file == NULL){
        err(EXIT_FAILURE, "Error opening in file %s\n", argv[2]);
    }
    
    /* Start processing file */
    while(EOF != (c = fgetc(in_file))){
        set_line_pos(&state, c);
        
        if(!format_break(&state, in_file, c)){ /* if not blank line */
            if(state.line_pos == START){ /* If start check for blocks */
                if(!(
                    format_hr(&state, in_file, c)      ||
                    format_header(&state, in_file, c)  || 
                    format_html(&state, in_file, c)    ||
                    format_ul_list(&state, in_file, c) ||
                    format_ol_list(&state, in_file, c) ||
                    format_code(&state, in_file, c)
                    )){

                    state.block = B_NONE;
                    fputs("<p>", stdout);
                    fputc(c, stdout);
                }
            } else if (state.line_pos == END){ /* If end check for closing blocks */
                close_blocks(&state);
                fputc(c, stdout);
            } else {
                fputc(c, stdout);
            }
        }
    }
    
    fprintf(stdout, "*****TEST Output complete*****\n");
    return 0;
}
