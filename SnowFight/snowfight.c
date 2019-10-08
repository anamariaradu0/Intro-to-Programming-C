// Copyright Radu Ana-Maria 2019
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure for the players
typedef struct player {
    char *name;
    int x;
    int y;
    int hp;
    int stamina;
    int gloves;
    int kills;
    int in_game;
} Player;

// Structure for the glacier
typedef struct glacier {
    int in_glacier;
    int occupant;
    int height;
    int gloves;
} Glacier;

// Function deciding whether a cell is inside the glacier or not,
// "dist" being the squared distance between where the elf lands and
// the center of the glacier
void is_cell_in_glacier(Glacier **map, int radius) {
    int ct = 2 * radius + 1;
    int dist, i, j;
    for (i = 0; i < ct; ++i) {
        for (j = 0; j < ct; ++j) {
            dist = (i - radius) * (i - radius) + (j - radius) * (j - radius);
            if (dist > radius * radius) {
                map[i][j].in_glacier = 0;
            } else {
                map[i][j].in_glacier = 1;
            }
        }
    }
}

// Function verifying if a player's coordinates are valid
int is_player_in_glacier(int x, int y, Glacier **map) {
    if (map[x][y].in_glacier == 0) {
        return 0;
    }
    return 1;
}

// Function that "kills" a player
void kill_player(int id, Player *elf) {
    elf[id].x = 0;
    elf[id].y = 0;
    elf[id].in_game = 0;
    elf[id].hp = 0;
    elf[id].stamina = 0;
    elf[id].gloves = 0;
}

// Function used for generating the fight between two elves
// player_1 is the player already occupying cell and player_2 is the one that
// comes to that particular cell
int  fight(Glacier **map, Player *elf, int player_1, int player_2, FILE *out) {
    int dmg_1 = elf[player_1].gloves;
    int dmg_2 = elf[player_2].gloves;

    if (elf[player_2].stamina <= elf[player_1].stamina) {
        while (1) {
            elf[player_2].hp -= dmg_1;
            if (elf[player_2].hp <= 0) {
                elf[player_1].stamina += elf[player_2].stamina;
                map[elf[player_1].x][elf[player_1].y].occupant = player_1;
                kill_player(player_2, elf);
                ++elf[player_1].kills;
                fprintf(out, "%s sent %s back home.\n",
                    elf[player_1].name, elf[player_2].name);
                return 0;
            }

            elf[player_1].hp -= dmg_2;
            if (elf[player_1].hp <= 0) {
                elf[player_2].stamina += elf[player_1].stamina;
                map[elf[player_2].x][elf[player_2].y].occupant = player_2;
                kill_player(player_1, elf);
                ++elf[player_2].kills;
                fprintf(out, "%s sent %s back home.\n",
                    elf[player_2].name, elf[player_1].name);
                return 0;
            }
        }
    }

    if (elf[player_2].stamina > elf[player_1].stamina) {
        while (1) {
            elf[player_1].hp -= dmg_2;
            if (elf[player_1].hp <= 0) {
                elf[player_2].stamina += elf[player_1].stamina;
                map[elf[player_2].x][elf[player_2].y].occupant = player_2;
                kill_player(player_1, elf);
                ++elf[player_2].kills;
                fprintf(out, "%s sent %s back home.\n",
                    elf[player_2].name, elf[player_1].name);
                return 0;
            }

            elf[player_2].hp -= dmg_1;
            if (elf[player_2].hp <= 0) {
                elf[player_1].stamina += elf[player_2].stamina;
                map[elf[player_1].x][elf[player_1].y].occupant = player_1;
                kill_player(player_2, elf);
                ++elf[player_1].kills;
                fprintf(out, "%s sent %s back home.\n",
                    elf[player_1].name, elf[player_2].name);
                return 0;
            }
        }
    }
    return 1;
}

// Function used for moving the player upwards
void move_up(Glacier **map, Player *elf, FILE *out, FILE *in,
        int num_players, int *in_game_players, int id, int radius_cp) {
    int dif = map[elf[id].x][elf[id].y].height -
    map[elf[id].x - 1][elf[id].y].height;

    if (elf[id].stamina >= abs(dif)) {
        map[elf[id].x][elf[id].y].occupant = -1;
        --elf[id].x;

        if ((elf[id].x - radius_cp) * (elf[id].x - radius_cp) +
            (elf[id].y - radius_cp) * (elf[id].y - radius_cp)
            > radius_cp * radius_cp) {
            --(*in_game_players);
            kill_player(id, elf);
            fprintf(out, "%s fell off the glacier.\n",
                elf[id].name);
        } else {
            elf[id].stamina -= abs(dif);

            if (map[elf[id].x][elf[id].y].gloves >
                    elf[id].gloves) {
                int aux = map[elf[id].x][elf[id].y].gloves;
                map[elf[id].x][elf[id].y].gloves = elf[id].gloves;
                elf[id].gloves = aux;
            }

            if (map[elf[id].x][elf[id].y].occupant >= 0 &&
                elf[map[elf[id].x][elf[id].y].occupant].in_game
                == 1) {
                --(*in_game_players);
                fight(map, elf,
                    map[elf[id].x][elf[id].y].occupant, id, out);
            } else {
                map[elf[id].x][elf[id].y].occupant = id;
            }
        }
    }
}

// Function used for moving the player downwards
void move_down(Glacier **map, Player *elf, FILE *out, FILE *in,
        int num_players, int *in_game_players, int id, int radius_cp) {
    int dif = map[elf[id].x][elf[id].y].height -
    map[elf[id].x + 1][elf[id].y].height;

    if (elf[id].stamina >= abs(dif)) {
        map[elf[id].x][elf[id].y].occupant = -1;
        ++elf[id].x;

        if ((elf[id].x - radius_cp) * (elf[id].x - radius_cp) +
            (elf[id].y - radius_cp) * (elf[id].y - radius_cp)
            > radius_cp * radius_cp) {
            --(*in_game_players);
            kill_player(id, elf);
            fprintf(out, "%s fell off the glacier.\n",
                elf[id].name);
        } else {
            elf[id].stamina -= abs(dif);

            if (map[elf[id].x][elf[id].y].gloves >
                    elf[id].gloves) {
                int aux = map[elf[id].x][elf[id].y].gloves;
                map[elf[id].x][elf[id].y].gloves = elf[id].gloves;
                elf[id].gloves = aux;
            }

            if (map[elf[id].x][elf[id].y].occupant >= 0 &&
                elf[map[elf[id].x][elf[id].y].occupant].in_game
                == 1) {
                --(*in_game_players);
                fight(map, elf, map[elf[id].x][elf[id].y].occupant,
                    id, out);

            } else {
                map[elf[id].x][elf[id].y].occupant = id;
            }
        }
    }
}

// Function used for moving the player to the left
void move_left(Glacier **map, Player *elf, FILE *out, FILE *in,
        int num_players, int *in_game_players, int id, int radius_cp) {
    int dif = map[elf[id].x][elf[id].y].height -
    map[elf[id].x][elf[id].y - 1].height;

    if (elf[id].stamina >= abs(dif)) {
        map[elf[id].x][elf[id].y].occupant = -1;
        --elf[id].y;

        if ((elf[id].x - radius_cp) * (elf[id].x - radius_cp) +
            (elf[id].y - radius_cp) * (elf[id].y - radius_cp)
            > radius_cp * radius_cp) {
            --(*in_game_players);
            kill_player(id, elf);
            fprintf(out, "%s fell off the glacier.\n",
                elf[id].name);
        } else {
            elf[id].stamina -= abs(dif);

            if (map[elf[id].x][elf[id].y].gloves > elf[id].gloves) {
                int aux = map[elf[id].x][elf[id].y].gloves;
                map[elf[id].x][elf[id].y].gloves = elf[id].gloves;
                elf[id].gloves = aux;
            }

            if (map[elf[id].x][elf[id].y].occupant >= 0 &&
                elf[map[elf[id].x][elf[id].y].occupant].in_game
                == 1) {
                --(*in_game_players);
                fight(map, elf, map[elf[id].x][elf[id].y].occupant,
                    id, out);
            } else {
                map[elf[id].x][elf[id].y].occupant = id;
            }
        }
    }
}

// Function used for moving the player to the right
void move_right(Glacier **map, Player *elf, FILE *out, FILE *in,
        int num_players, int *in_game_players, int id, int radius_cp) {
    int dif = map[elf[id].x][elf[id].y].height -
    map[elf[id].x][elf[id].y + 1].height;

    if (elf[id].stamina >= abs(dif)) {
        map[elf[id].x][elf[id].y].occupant = -1;
        ++elf[id].y;

        if ((elf[id].x - radius_cp) * (elf[id].x - radius_cp) +
            (elf[id].y - radius_cp) * (elf[id].y - radius_cp)
            > radius_cp * radius_cp) {
            --(*in_game_players);
            kill_player(id, elf);
            fprintf(out, "%s fell off the glacier.\n",
                elf[id].name);
        } else {
            elf[id].stamina -= abs(dif);

            if (map[elf[id].x][elf[id].y].gloves >
                elf[id].gloves) {
                int aux = map[elf[id].x][elf[id].y].gloves;
                map[elf[id].x][elf[id].y].gloves = elf[id].gloves;
                elf[id].gloves = aux;
            }

            if (map[elf[id].x][elf[id].y].occupant >= 0 &&
                elf[map[elf[id].x][elf[id].y].occupant].in_game
                == 1) {
                --(*in_game_players);
                fight(map, elf, map[elf[id].x][elf[id].y].occupant,
                    id, out);
            } else {
                map[elf[id].x][elf[id].y].occupant = id;
            }
        }
    }
}

// Function used for moving a player on the map
void move_player(Glacier **map, Player *elf, FILE *out, FILE *in,
        int num_players, int *in_game_players, int radius_cp) {
    int id, dif;
    char direction;

    fscanf(in, " %d ", &id);

    while (fscanf(in, "%c", &direction)
        && direction != '\n'
        && direction != EOF) {
        if (elf[id].in_game == 1) {
            if (direction == 'U') {
                move_up(map, elf, out, in,
                    num_players, in_game_players, id, radius_cp);
            }
            if (direction == 'D') {
                move_down(map, elf, out, in,
                    num_players, in_game_players, id, radius_cp);
            }
            if (direction == 'L') {
                move_left(map, elf, out, in,
                    num_players, in_game_players, id, radius_cp);
            }
            if (direction == 'R') {
                move_right(map, elf, out, in,
                    num_players, in_game_players, id, radius_cp);
            }
        }
    }
}

// Function used for generating the snowstorm
void snowstorm(Glacier **map, Player *elf, int num_players,
    FILE *in, FILE *out, int *in_game_players) {
    long k;
    int s_x, s_y, s_radius, s_damage, i;

    fscanf(in, " %ld", &k);

    // Extracting the parameters of the storm by shifting the given input
    s_x = k & 255;
    k >>= 8;
    s_y = k & 255;
    k >>= 8;
    s_radius = k & 255;
    k >>= 8;
    s_damage = k & 255;

    int s_dist = 0;
    for (i = 0; i < num_players; ++i) {
        s_dist = (elf[i].x - s_x) * (elf[i].x - s_x) +
            (elf[i].y - s_y) * (elf[i].y - s_y);
        if (s_dist <= s_radius * s_radius) {
            elf[i].hp -= s_damage;
            if (elf[i].hp <= 0 && elf[i].in_game == 1) {
                kill_player(i, elf);
                --(*in_game_players);
                fprintf(out, "%s was hit by snowstorm.\n", elf[i].name);
            }
        }
    }
}

// Function generating the meltdown
Glacier** meltdown(Glacier **map, int radius, int radius_cp, Player *elf,
    FILE *in, FILE *out, int num_players, int *in_game_players) {
    int stamina, i, j;
    fscanf(in, " %d", &stamina);

    int first_ct = 2 * radius + 1;
    int final_ct = 2 * radius_cp + 1;

    // Allocating memory for a smaller, auxiliary map
    Glacier **aux_map;
    aux_map = (Glacier**)calloc(final_ct, sizeof(Glacier*));
    for (i = 0; i < final_ct; ++i) {
        aux_map[i] = (Glacier*)calloc(final_ct, sizeof(Glacier));
    }

    // Verifying which cells of the new map are on the glacier
    for (i = 0; i < final_ct; ++i) {
        for (j = 0; j < final_ct; ++j) {
            if ( (i - radius_cp) * (i - radius_cp) +
                (j - radius_cp) * (j - radius_cp)
                <= radius_cp * radius_cp) {
                aux_map[i][j].in_glacier = 1;

            } else {
                aux_map[i][j].in_glacier = 0;
            }
        }
    }

    // Deciding which elves survive the global warming by moving them towards
    // the upper left corner of the matrix
    for (i = 0; i < num_players; ++i) {
        if (elf[i].in_game == 1) {
        elf[i].x--;
        elf[i].y--;
        elf[i].stamina += stamina;
            if ((elf[i].x - radius_cp) * (elf[i].x - radius_cp) +
                (elf[i].y - radius_cp) * (elf[i].y - radius_cp)
                > radius_cp * radius_cp) {
            fprintf(out, "%s got wet because of global warming.\n",
            elf[i].name);
            kill_player(i, elf);
            --(*in_game_players);
         }
       }
    }

    // Copying contents from the auxiliary map to the new map
    for (i = 0; i < final_ct; ++i) {
        for (j = 0; j < final_ct; ++j) {
            memcpy(&aux_map[i][j], &map[i+1][j+1], sizeof(Glacier));
        }
    }

    // Freeing memory from the last map
    for (i = 0; i < first_ct; ++i) {
        free(map[i]);
    }
    free(map);

    return aux_map;
}

// Function that prints the scoreboard
void print_scoreboard(Glacier **map, Player *elf, FILE *in, FILE *out,
    int num_players, int *in_game_players) {

    int i, j;

    Player *elf_copy_wet = calloc(num_players, sizeof (Player));
    Player *elf_copy_dry = calloc(num_players, sizeof(Player));
    int wet = 0, dry = 0;

    // Separating the elves into 'DRY' and 'WET'
    for (i = 0; i < num_players; ++i) {
        if (elf[i].in_game == 1) {
            elf_copy_dry[dry] = elf[i];
            ++dry;
        } else {
            elf_copy_wet[wet] = elf[i];
            ++wet;
        }
    }

    // Sorting the 'DRY' elves by number of kills and name
    for (i = 0; i < dry - 1; ++i) {
        for (j = i + 1; j < dry; ++j) {
            if (elf_copy_dry[i].kills < elf_copy_dry[j].kills) {
                Player elf_aux = elf_copy_dry[i];
                elf_copy_dry[i] = elf_copy_dry[j];
                elf_copy_dry[j] = elf_aux;
            } else if (elf_copy_dry[i].kills == elf_copy_dry[j].kills) {
                if (strcmp(elf_copy_dry[i].name, elf_copy_dry[j].name) > 0) {
                    Player elf_aux = elf_copy_dry[i];
                    elf_copy_dry[i] = elf_copy_dry[j];
                    elf_copy_dry[j] = elf_aux;
                }
            }
        }
    }

    // Sorting the 'WET' elves by number of kills and name
    for (i = 0; i < wet - 1; ++i) {
        for (j = i + 1; j < wet; ++j) {
            if (elf_copy_wet[i].kills < elf_copy_wet[j].kills) {
                Player elf_aux = elf_copy_wet[i];
                elf_copy_wet[i] = elf_copy_wet[j];
                elf_copy_wet[j] = elf_aux;
            } else if (elf_copy_wet[i].kills == elf_copy_wet[j].kills) {
                if (strcmp(elf_copy_wet[i].name, elf_copy_wet[j].name) > 0) {
                    Player elf_aux = elf_copy_wet[i];
                    elf_copy_wet[i] = elf_copy_wet[j];
                    elf_copy_wet[j] = elf_aux;
                }
            }
        }
    }

    // Printing the list as requested
    fprintf(out, "SCOREBOARD:\n");
    for (i = 0; i < dry; ++i) {
        fprintf(out, "%s\tDRY\t%d\n", elf_copy_dry[i].name,
            elf_copy_dry[i].kills);
    }
    for (i = 0; i < wet; ++i) {
        fprintf(out, "%s\tWET\t%d\n", elf_copy_wet[i].name,
            elf_copy_wet[i].kills);
    }

    free(elf_copy_dry);
    free(elf_copy_wet);
}

// Function that verifies if there is a winner
int verify_winner(int *in_game_players, int num_players,
    Player *elf, FILE *out, Glacier **map, FILE *in, int radius) {
    int i;

    if (*in_game_players == 1) {
        for (i = 0; i < num_players; ++i) {
            if (elf[i].in_game == 1) {
                fprintf(out, "%s has won.\n", elf[i].name);
                for (i = 0; i < 2 * radius + 1; ++i) {
                    free(map[i]);
                }
                for (i = 0; i < num_players; ++i) {
                    free(elf[i].name);
                }
                 free(map);
                 free(elf);
                 fclose(in);
                 fclose(out);
                 return 1;
            }
        }
    }
    return 0;
}

// Initializing all the occupants with -1
void initialise_occupants(Glacier **map, Player *elf, int radius) {
    int i, j, ct;
    ct = 2 * radius + 1;
    for (i = 0; i < ct; ++i) {
        for (j = 0; j < ct; ++j) {
            map[i][j].occupant = -1;
        }
    }
}

// Reading the characteristics of the map
void read_map(Glacier **map, int ct, FILE *in) {
    int i, j;
    for (i = 0; i < ct; ++i) {
        for (j = 0; j < 2 * ct; j += 2) {
            fscanf(in, "%d", &map[i][j / 2].height);
            fscanf(in, "%d", &map[i][(j + 1) / 2].gloves);
        }
    }
}

void free_memory(Glacier **map, Player *elf, int radius, int num_players) {
    int i, j;

    for (i = 0; i < 2 * radius + 1; ++i) {
        free(map[i]);
    }
    free(map);

    for (i = 0; i < num_players; ++i) {
        free(elf[i].name);
    }
    free(elf);
}

// Main function
int main() {
    char in_file[] = "snowfight.in", out_file[] = "snowfight.out", *command;

    // Opening the input file
    FILE *in = fopen(in_file, "r"), *out = fopen(out_file, "w");
    if (in == NULL) {
        fprintf(stderr, "Can't open file.\n");
        return -1;
    }

    // Reading the radius and number of players
    int radius, num_players, i, j;

    fscanf(in, "%d %d", &radius, &num_players);
    int in_game_players = num_players;

    int ct = 2 * radius + 1, radius_cp = radius;

    // Reading the characteristics of the map
    Glacier **map;
    map = (Glacier**)calloc(ct, sizeof (Glacier *));
    for (i = 0; i < ct; ++i) {
        map[i] =(Glacier*)calloc(ct, sizeof (Glacier));
    }
    read_map(map, ct, in);

    // Deciding which cells are inside the glacier
    is_cell_in_glacier(map, radius);

    // Reading the initial characteristics of each elf and adding -1 to the
    // initial map for occupants
    Player *elf;
    elf = calloc(num_players, sizeof (Player));

    initialise_occupants(map, elf, radius);

    for (i = 0; i < num_players; ++i) {
        elf[i].name =  (char *)malloc(20 * sizeof(char));
        fscanf(in, "%s %d %d %d %d", elf[i].name, &elf[i].x,
            &elf[i].y, &elf[i].hp, &elf[i].stamina);
        if (map[elf[i].x][elf[i].y].in_glacier == 0) {
            elf[i].in_game = 0;
            fprintf(out, "%s has missed the glacier.\n", elf[i].name);
            --in_game_players;
        } else {
            elf[i].in_game = 1;
            if (verify_winner(&in_game_players, num_players, elf,
                out, map, in, radius)) {
                return 0;
            }
            elf[i].kills = 0;
            elf[i].gloves = map[elf[i].x][elf[i].y].gloves;
            map[elf[i].x][elf[i].y].gloves = 0;
            map[elf[i].x][elf[i].y].occupant = i;
        }
    }

    // Reading each command until the end of the file
    // 18 was chosen for the command because the longest possible string has
    // 17 characters
    command = (char *)malloc (18* sizeof(char));

    while (fscanf(in, "%s", command) != -1 && in_game_players != 1) {
        if (strcmp(command, "MOVE") == 0) {
            move_player(map, elf, out, in, num_players,
                &in_game_players, radius);
            if (verify_winner(&in_game_players, num_players, elf,
                out, map, in, radius)) {
                free(command);
                return 0;
            }
        } else if (strcmp(command, "SNOWSTORM") == 0) {
            snowstorm(map, elf, num_players, in, out, &in_game_players);
            if (verify_winner(&in_game_players, num_players, elf,
                out, map, in, radius)) {
                free(command);
                return 0;
            }
        } else if (strcmp(command, "MELTDOWN") == 0) {
            --radius_cp;
            map = meltdown(map, radius, radius_cp, elf, in,
            out, num_players, &in_game_players);
            radius = radius_cp;
            if (verify_winner(&in_game_players, num_players, elf,
                out, map, in, radius)) {
                free(command);
                return 0;;
            }
        } else if (strcmp(command, "PRINT_SCOREBOARD") == 0) {
            print_scoreboard(map, elf, in, out, num_players, &in_game_players);
        }
        memset(command, 0, 18);
    }

    // Freeing the used memory
    free(command);
    free_memory(map, elf, radius, num_players);
    fclose(in);
    fclose(out);

    return 0;
}
