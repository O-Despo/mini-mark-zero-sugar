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

int m_fgetc(State *state, FILE *in_file, int c);
int m_ungetc(State *state, FILE *in_file, int c);
short format_break(State *state, FILE *in_file, char c);
short format_html(State *state, FILE *in_file, char c);
short format_code(State *state, FILE *in_file, char c);
short format_header(State *state, FILE *in_file, char c);
short format_hr(State *state, FILE *in_file, char c);
short format_ul_list(State *state, FILE *in_file, char c);
short format_ul_list(State *state, FILE *in_file, char c);
short in_line_emphasis_bold(State *state, FILE *in_file, char c);
short in_line_code(State *state, FILE *in_file, char c);
short escape(State *state, FILE *in_file, char c);
short in_line_link(State *state, FILE *in_file, char c);
short close_blocks(State *state);
