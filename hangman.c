#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LENGTH 13
#define MAX_VIEW_LINES 7
#define MAX_VIEW_LINE_LENGTH 50

typedef struct {
    int gameover;
    int wrong_guesses;
    char dictionary[50];
    char letters_guessed[26];
    char current_word[MAX_WORD_LENGTH];
    char word[MAX_WORD_LENGTH + 1];
    char view[MAX_VIEW_LINES][MAX_VIEW_LINE_LENGTH];
} Hangman;

void initialize_view(Hangman *hangman) {
    FILE *file = fopen("display.txt", "r");
    if (file != NULL) {
        for (int i = 0; i < MAX_VIEW_LINES; i++) {
            if (fgets(hangman->view[i], sizeof(hangman->view[i]), file) == NULL) {
                // Handle case where the file does not contain enough lines
                // or an error occurs while reading the file
                fprintf(stderr, "Error reading display.txt\n");
                exit(EXIT_FAILURE);
            }
            hangman->view[i][strcspn(hangman->view[i], "\n")] = '\0';  // Remove trailing newline
        }
        fclose(file);
    } else {
        // Handle case where the file cannot be opened
        fprintf(stderr, "Error opening display.txt\n");
        exit(EXIT_FAILURE);
    }
}

void print_status(Hangman *hangman) {
    printf("\nThe current word is: %s\n", hangman->current_word);
    printf("%s", hangman->view[hangman->wrong_guesses]);
    printf("\nLetters you have guessed: %s\n", hangman->letters_guessed);
    printf("You have %d lives left\n", 6 - hangman->wrong_guesses);
}

void get_input(Hangman *hangman) {
    char guess[2];
    int valid_guess;
    do {
        printf("Guess a letter or type 'solve' to solve: ");
        fgets(guess, sizeof(guess), stdin);
        guess[strcspn(guess, "\n")] = '\0';  // Remove trailing newline
        valid_guess = is_valid(hangman, guess);
    } while (!valid_guess);
}

int is_valid(Hangman *hangman, char *guess) {
    if (strcmp(guess, "SOLVE") == 0) {
        return 1;
    } else if (strchr(hangman->letters_guessed, guess[0]) != NULL) {
        return 0;
    } else if (strlen(guess) != 1) {
        return 0;
    } else if (isalpha(guess[0])) {
        hangman->letters_guessed[strlen(hangman->letters_guessed)] = toupper(guess[0]);
        return 1;
    } else {
        return 0;
    }
}

void determine_outcome(Hangman *hangman) {
    if (strchr(hangman->word, hangman->letters_guessed[strlen(hangman->letters_guessed) - 1]) != NULL) {
        for (int i = 0; i < strlen(hangman->word); i++) {
            if (hangman->letters_guessed[strlen(hangman->letters_guessed) - 1] == hangman->word[i]) {
                hangman->current_word[i] = hangman->word[i];
            }
        }
    } else {
        hangman->wrong_guesses++;
    }
}

int is_gameover(Hangman *hangman) {
    if (hangman->wrong_guesses >= 6) {
        hangman->gameover = 1;
    } else {
        for (int i = 0; i < strlen(hangman->current_word); i++) {
            if (hangman->current_word[i] == '*') {
                return 0;
            }
        }
    }
    return 1;
}

void solve_attempt(Hangman *hangman) {
    char attempt[MAX_WORD_LENGTH];
    printf("Enter the solution to the word: ");
    fgets(attempt, sizeof(attempt), stdin);
    attempt[strcspn(attempt, "\n")] = '\0';  // Remove trailing newline
    if (strcmp(attempt, hangman->word) == 0) {
        hangman->gameover = 1;
        strcpy(hangman->current_word, hangman->word);
        printf("You guessed the word correctly!\n");
    } else {
        hangman->wrong_guesses++;
        printf("Sorry, that's incorrect!\n");
    }
}

void print_result(Hangman *hangman) {
    if (hangman->wrong_guesses >= 6) {
        printf("\n\n=======================================\n");
        printf("============== You lose! ==============\n");
        printf("=======================================\n");
    } else {
        printf("\n\n=======================================\n");
        printf("====== Congratulations, you win! =======\n");
        printf("=======================================\n");
    }
}

void load_game(Hangman *hangman) {
    FILE *file = fopen("saves/save_file.txt", "r");
    if (file != NULL) {
        fread(hangman, sizeof(Hangman), 1, file);
        fclose(file);
    } else {
        fprintf(stderr, "ERROR! There is no save file!\n");
    }
}

void save_game(Hangman *hangman) {
    mkdir("saves");
    FILE *file = fopen("saves/save_file.txt", "w");
    if (file != NULL) {
        fwrite(hangman, sizeof(Hangman), 1, file);
        fclose(file);
    }
}

char *get_word(char *dictionary) {
    static char word[MAX_WORD_LENGTH + 1];
    FILE *file = fopen(dictionary, "r");
    if (file != NULL) {
        char words[100][MAX_WORD_LENGTH + 1];
        int count = 0;
        while (fgets(words[count], sizeof(words[count]), file) != NULL) {
            words[count][strcspn(words[count], "\n")] = '\0';  // Remove trailing newline
            count++;
        }
        fclose(file);

        int index;
        do {
            index = rand() % count;
        } while (strlen(words[index]) <= 5 || strlen(words[index]) > 13);

        strncpy(word, words[index], MAX_WORD_LENGTH);
        word[MAX_WORD_LENGTH] = '\0';
    }
    return word;
}

int main(void) {
    char to_load[2];
    printf("Do you want to load the game (Y/N): ");
    fgets(to_load, sizeof(to_load), stdin);
    to_load[strcspn(to_load, "\n")] = '\0';  // Remove trailing newline

    Hangman hangman;
    if (strcmp(to_load, "Y") == 0) {
        load_game(&hangman);
    } else {
        strcpy(hangman.dictionary, "dictionary.txt");
        hangman.gameover = 0;
        hangman.wrong_guesses = 0;
        hangman.letters_guessed[0] = '\0';
        hangman.current_word[0] = '\0';
        hangman.word[0] = '\0';
        initialize_view(&hangman);  // Initialize hangman.view
        strncpy(hangman.current_word, get_word(hangman.dictionary), MAX_WORD_LENGTH);
    }

    while (!hangman.gameover) {
        print_status(&hangman);
        get_input(&hangman);
        if (strcmp(hangman.letters_guessed, "SOLVE") == 0) {
            solve_attempt(&hangman);
        } else {
            determine_outcome(&hangman);
        }
        hangman.gameover = is_gameover(&hangman);
        save_game(&hangman);
    }

    print_result(&hangman);

    return 0;
}
