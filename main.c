#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define bool int
#define TRUE 1
#define FALSE 0
#define QUIT -1

struct Card {
    char *question;
    char *answer;
};

int beginning;
int ending;
int length;
struct Card **cards;
int cycles;

int MAX_LINE_LENGTH = 100;
int MAX_PHRASE_LENGTH = 70;

int real_len(char *str) {
    int length = strlen(str);
    int i;
    char *j;
    for (i = 0; i < strlen(str); i++) {
        if ((int)str[i] == -61) {
            length--;
        }
    }
    return length;
}

int get_columns() {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return w.ws_col;
}

int get_rows() {
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return w.ws_row;
}

int mod(const int val1, const int val2) {
    int result = val1;
    if (val1 < 0) {
        while (result < 0) {
            result += val2;
        }
        return result;
    } else {
        while (result > val2) {
            result -= val2;
        }
        return result;
    }
}

int max(const int val1, const int val2) {
    if (val1 > val2) {
        return val1;
    } else {
        return val2;
    }
}

void print_horizontal_line() {
    int i;
    for (i = get_columns(); i > 0; i--) {
        printf("-");
    }
    printf("\n");
}

void clear_input() {
    char c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

// Validate arguments.
int validate_arguments(int argc, char **argv) {
    if (argc == 2 && strcmp(argv[1], "help") == 0) {
        printf(
            "\n"
            "FORMAT:\n"
            "./a.out [path] [start] [end] [length]\n"
            "\n"
            "ARGUMENTS:\n"
            "[path]: the file path to the target .csv\n"
            "[start]: which line to start reading from (inclusive).\n"
            "[end]: which line to stop reading (inclusive).\n"
            "\n"
            "OPTIONAL ARGUMENT [length]:\n"
            "  - The default character limit for questions and answers is 70 each\n"
            "  - The default character limit for each line (question + answer) is 100\n"
            "  - Passing \"long\" as the [length] argument doubles both limits.\n"
            "  - Passing \"superlong\" as the [length] argument quadruples both limits.\n"
            "\n"
            "The target csv should be formatted as such:\n"
            "  question1,answer1\n"
            "  question2,answer2\n"
            "  question3,answer3\n"
            "  ...\n"
            "\n"
            "An example:\n"
            "  acceleration,acceleración\n"
            "  10 minutes away,a 10 minutos\n"
            "  unknown,desconocido/a\n"
            "  ...\n"
            "\n"
            "Special and accented characters are accepted. Commas are not accepted in the question segment and will result in an unreported parsing error, but allowed in the answer segment.\n\n"
            "Submit \"q\" as an answer to any question to quit immediately.\n\n"
        );
        return FALSE;
    }

    if (argc != 4 && argc != 5) {
        printf("Error: Too many or too few arguments; should be 3.\n\n");
        return FALSE;
    }

    if (argc == 5) {
        if (strcmp(argv[4], "long") == 0) {
            MAX_LINE_LENGTH = 200;
            MAX_PHRASE_LENGTH = 140;
        } else if (strcmp(argv[4], "superlong") == 0) {
            MAX_LINE_LENGTH = 400;
            MAX_PHRASE_LENGTH = 280;
        } else {
            printf("Error: Invalid length specifier \"%s\"\n\n", argv[4]);
            return FALSE;
        }
    }

    beginning = atoi(argv[2]) - 1;
    ending = atoi(argv[3]) - 1;
    length = ending - beginning + 1;

    if (ending < beginning) {
        printf("Error: Invalid end position: less than start position.\n\n");
        return FALSE;
    }

    if (beginning < 0) {
        printf("Error: Invalid beginning position: less than 1.\n\n");
        return FALSE;
    }

    return TRUE;
}

// Loads cards from line 'beginning' to line 'ending' of given file.
int load_cards(FILE *file) {
    int i;
    char gotten[MAX_LINE_LENGTH];

    // Allocate memory
    cards = malloc(sizeof(struct Card *) * length);
    for (i = 0; i < length; i++) {
        cards[i] = malloc(sizeof(struct Card));
        cards[i]->question = malloc(sizeof(char) * MAX_PHRASE_LENGTH);
        cards[i]->answer = malloc(sizeof(char) * MAX_PHRASE_LENGTH);
    }

    struct Card *card;  // Current card being loaded, just for readability.

    for (i = 0; fgets(gotten, MAX_LINE_LENGTH, file) != NULL; i++) {
        if (i < beginning) {  // Skip lines until we reach the beginning.
            continue;
        }

        card = cards[i - beginning];

        strtok(gotten, "\n\r");  // Truncates the \n at the end.
        char *part1 = strtok(gotten, ",");
        char *part2 = strtok(NULL, "");

        if (part1 == NULL || part2 == NULL) {  // Verifies tokens
            printf(
                "Error: Invalid file contents: parsing failure at line %d\n\n",
                (i + 1));
            return FALSE;
        }

        strcpy(card->question, part1);  // Copies verified tokens
        strcpy(card->answer, part2);

        if (i >= ending) {  // If we have passed the end, stop.
            break;
        }
    }

    if (i < beginning) {  // This means that 'beginning' > number of lines
        printf("Error: Beginning position exceeds file length.\n\n");
        return FALSE;
    } else if (i != ending) {  // This means that 'ending' > number of lines
        printf(
            ANSI_COLOR_YELLOW
            "Warning: Ending position \"%d\" exceeds file length (%d lines). "
            "Proceed? Y/n " ANSI_COLOR_RESET,
            ending+1, i);
        if (getc(stdin) ==
            'n') {  // Returns false to cancel operation if user says 'no'
            printf("\n");
            return FALSE;
        }
        length = i - beginning;
        cards = realloc(cards, sizeof(struct Card *) * (i - beginning));
    }

    return TRUE;
}

// Self-explantorily shuffles the global variable 'cards'.
void shuffle(struct Card **deck, int size) {
    srand(time(NULL));
    int i;
    int j;
    struct Card *tmp;
    for (i = size - 1; i >= 0; i--) {
        j = rand() % (i + 1);
        tmp = deck[i];
        deck[i] = deck[j];
        deck[j] = tmp;
    }
}

int test_card(struct Card **cards, int i, int size) {
    struct Card card = *(cards[i]);

    // Set up counters so we right-align "[i/size]" later.
    int to_be_filled = get_columns();

    // Print preface
    int preface_length = printf(" %12s -> ", card.question);

    // Obtain response
    char *response = malloc(sizeof(char) * (MAX_PHRASE_LENGTH+1));
    fgets(response, MAX_PHRASE_LENGTH+1, stdin);
    strtok(response, "\r\n");
    if(strlen(response)==MAX_PHRASE_LENGTH){
        clear_input(); //Clears input buffer if user overflowed MAX_PHRASE_LENGTH
    }

    // If response is "q", immediately quit
    if (strcmp(response, "q") == 0) {
        return QUIT;
    }

    // If response is "\s", skip and remove from deck
    if (strcmp(response, "\\s") == 0) {
        printf("\n\n");
        free(response);
        return TRUE;
    }

    // Creates a blank string of minimum length 12 for alignment
    int length = max(12, real_len(card.question)) + 1;
    char *space = malloc(sizeof(char) * length);
    memset(space, ' ', length - 1);
    space[length - 1] = '\0';

    // Display the solution
    bool result;
    int compared = strcmp(response, card.answer);
    char *solution = malloc(sizeof(char) * (length + strlen(card.answer) + 26));
    // 38 = 5 (  -> ) + 4*5 (4*strlen(ANSI_COLOR)).
    // The space for '\0' is included in 'length'.

    if (compared == 0) {
        // If completely correct, highlight with green
        sprintf(solution, ANSI_COLOR_GREEN " %s -> %s" ANSI_COLOR_RESET, space,
                card.answer);
        printf("%s", solution);
        to_be_filled -= real_len(solution) - strlen(ANSI_COLOR_GREEN) -
                        strlen(ANSI_COLOR_RESET);
        result = TRUE;
    } else {
        char *substring_pos = strstr(card.answer, response);
        if (substring_pos == NULL) {
            // If completely wrong, highlight with red.
            sprintf(solution, ANSI_COLOR_RED " %s -> %s" ANSI_COLOR_RESET,
                    space, card.answer);
            printf("%s", solution);
            to_be_filled -= real_len(solution) - strlen(ANSI_COLOR_RED) -
                            strlen(ANSI_COLOR_RESET);
        } else {
            /// If partially correct, highlight yellow the incorrect portion.
            const int head_length = substring_pos - card.answer;
            const int tail_length =
                strlen(card.answer) - head_length - strlen(response);
            char *head = malloc(sizeof(char) * head_length);
            strncpy(head, card.answer, head_length);
            head[head_length] = '\0';
            char *tail = substring_pos + strlen(response);
            sprintf(solution,
                    ANSI_COLOR_YELLOW " %s -> %s" ANSI_COLOR_RESET
                                      "%s" ANSI_COLOR_YELLOW
                                      "%s" ANSI_COLOR_RESET,
                    space, head, response, tail);
            printf("%s", solution);
            to_be_filled -= real_len(solution) - strlen(ANSI_COLOR_YELLOW) * 2 -
                            strlen(ANSI_COLOR_RESET) * 2;
            free(head);
        }
        result = FALSE;
    }

    // Print "[i/size]"
    char *question_num = malloc(sizeof(char) * 11);
    to_be_filled -= sprintf(question_num, "[%d/%d] ", i + 1, size);
    to_be_filled = mod(to_be_filled, get_columns());
    char *alignment_fill = malloc(sizeof(char) * (to_be_filled + 1));
    memset(alignment_fill, ' ', to_be_filled);
    alignment_fill[to_be_filled] = '\0';
    printf("%s%s\n\n", alignment_fill, question_num);

    // Free memory and return
    free(solution);
    free(question_num);
    free(alignment_fill);
    free(space);
    free(response);
    return result;
}

// The main loop.
void loop() {
    // Initialize a full deck.
    int deck_size = length;
    int incorrect = 0;
    int first_round_incorrect;
    struct Card **deck = malloc(sizeof(struct Card *) * length);
    int i;
    for (i = 0; i < deck_size; i++) {
        deck[i] = cards[i];
    }

    // Keep dealing the deck until it is empty.
    while (deck_size > 0) {
        // Shuffle
        shuffle(deck, deck_size);

        // Test the player
        for (i = 0; i < deck_size; i++) {
            int test_result = test_card(deck, i, deck_size);
            if (test_result == TRUE) {  // If response is correct
                deck[i] = NULL;         // Remove card from deck
            } else if (test_result == QUIT) {
                return;  // Stop loop if player cancels
            }            // Implicitly keep card in deck if incorrect
        }
        // Collapse the removed cards
        int removed = 0;
        for (i = 0; i < deck_size; i++) {
            if (deck[i] == NULL) {
                removed++;
            } else {
                deck[i - removed] = deck[i];
            }
        }
        // Update the deck size and error count to reflect removed cards.
        if (deck_size == length) {
            first_round_incorrect = removed;
        }
        deck_size -= removed;
        incorrect += removed;
    }

    cycles++;
    printf(
        "\n------------------------------------------------------------------\n"
        "  You have completed %d cycle%s of the selected deck (%d cards).\n"
        "  This cycle, you got %d%% of the terms right the first time."
        "\n------------------------------------------------------------------\n"
        "\n\n",
        cycles, cycles == 1 ? "" : "s", cycles * length,
        (int)roundf(((float)first_round_incorrect) / length * 100));
    
    free(deck);
    
    loop();
}

// Arguments taken are: flashcards <filepath> <startindex> <endindex>
int main(int argc, char **argv) {
    if (validate_arguments(argc, argv) ==
        FALSE) {  // Returns false for a variety of reasons.
        return 1;
    }

    FILE *file;
    file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Invalid file path: File not found.\n\n");
        return 1;
    }

    if (load_cards(file) == FALSE) {  // Returns false if parsing error.
        return 1;
    }

    system("@cls||clear");
    // Success report.
    print_horizontal_line();
    printf("\n%d cards successfully loaded.\n\n", length);
    printf("  %s / %s\n", cards[0]->question, cards[0]->answer);
    printf("   ... \n   ... \n   ... \n");
    printf("  %s / %s\n\n", cards[length - 1]->question,
           cards[length - 1]->answer);
    print_horizontal_line();
    printf("\n\n");

    // Begins flashcard loop
    loop();

    // Clears screen
    system("@cls||clear");

    return 0;
}
