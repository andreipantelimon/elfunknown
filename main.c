// COPYRIGHT 2018 Andrei Pantelimon
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
// Structura playerilor (elfii) cu id, coordonate, hp, stamina,
// un ok daca inca este in joc, numarul de castiguri, dmg si numele
struct elfs{
    int id;
    int x;
    int y;
    int hp;
    int stamina;
    int ok;
    int kills;
    int dmg;
    char nume[20];
};
// Structura ghetarului cu altitudine, manusile din fiecare celula si
// un contor daca este ocupata celula de un elf
struct ices{
    int alt;
    int man;
    int busy;
};
// Functia de bataie
void fight(FILE *out, struct elfs *elf, struct ices **ice,
    int id1, int x2, int y2, int P) {
    int i = 0, id2 = 0;
    // Se gaseste elful care este "gazda"
    for (i = 0; i < P; i++) {
        if (elf[i].x == x2 && elf[i].y == y2 && i != id1 && elf[i].hp >= 0) {
            id2 = i;
        }
    }
    // Algoritmul de bataie in care ataca "musafirul"
    if (elf[id1].ok == 1 && elf[id2].ok == 1  &&
        (elf[id1].dmg != 0 || elf[id2].dmg != 0)) {
        if (elf[id1].stamina >= elf[id2].stamina) {
            while (1) {
                elf[id2].hp -= elf[id1].dmg;
                if (elf[id2].hp <= 0) {
                    fprintf(out, "%s sent %s back home.\n",
                        elf[id1].nume, elf[id2].nume);
                    ice[elf[id2].x][elf[id2].y].busy = 1;
                    elf[id2].ok = 0;
                    elf[id1].stamina += elf[id2].stamina;
                    elf[id2].stamina = 0;
                    elf[id1].kills++;
                    break;
                }
                elf[id1].hp -= elf[id2].dmg;
                if (elf[id1].hp <= 0) {
                    fprintf(out, "%s sent %s back home.\n",
                        elf[id2].nume, elf[id1].nume);
                    ice[elf[id1].x][elf[id1].y].busy = 1;
                    elf[id1].ok = 0;
                    elf[id2].stamina += elf[id1].stamina;
                    elf[id1].stamina = 0;
                    elf[id2].kills++;
                    break;
                }
            }
            // Algoritmul in care ataca "gazda"
        } else {
            while (1) {
                elf[id1].hp -= elf[id2].dmg;
                if (elf[id1].hp <= 0) {
                    fprintf(out, "%s sent %s back home.\n",
                        elf[id2].nume, elf[id1].nume);
                    ice[elf[id1].x][elf[id1].y].busy = 1;
                    elf[id1].ok = 0;
                    elf[id2].stamina += elf[id1].stamina;
                    elf[id1].stamina = 0;
                    elf[id2].kills++;
                    break;
                }
                elf[id2].hp -= elf[id1].dmg;
                if (elf[id2].hp <= 0) {
                    fprintf(out, "%s sent %s back home.\n",
                        elf[id1].nume, elf[id2].nume);
                    ice[elf[id2].x][elf[id2].y].busy = 1;
                    elf[id2].ok = 0;
                    elf[id1].stamina += elf[id2].stamina;
                    elf[id2].stamina = 0;
                    elf[id1].kills++;
                    break;
                }
            }
        }
    }
}
// Functia care realizeaza distanta in plan, centrul ramanand
// acelasi tot timpul
int dropzone(int x, int y, int R, int R_orig) {
    float d = 0;
    int xc = R_orig, yc = R_orig;
    d = sqrt((xc - x) * (xc - x) + (yc - y) * (yc - y));
    if (d <= R) {
        return 1;
    } else {
        return 0;
    }
}
// Functia de citire
void citire(struct ices **ice, struct elfs *elf, int R,
    int P, FILE *in, FILE *out) {
    int i = 0, j = 0;
    int A = 0, B = 0;
    int x = 0, y = 0, stam = 0, hp = 0;
    for (i = 0; i < 2 * R + 1; i++) {
        for (j = 0; j < 2 * R + 1; j++) {
            fscanf(in, "%d%d", &A, &B);
            ice[i][j].alt = A;
            ice[i][j].man = B;
        }
    }
    for (i = 0; i < 2 * R + 1; i++) {
       for (j = 0; j < 2 * R + 1; j++) {
           ice[i][j].busy = 0;
       }
    }
    for (i = 0; i < P; i++) {
        elf[i].kills = 0;
    }
    for (i = 0; i < P; i++) {
        fscanf(in, "%s%d%d%d%d", elf[i].nume, &x, &y, &hp, &stam);
        elf[i].x = x;
        elf[i].y = y;
        elf[i].hp = hp;
        elf[i].stamina = stam;
        elf[i].id = i;
        ice[x][y].busy = 1;
        elf[i].dmg = ice[elf[i].x][elf[i].y].man;
        ice[elf[i].x][elf[i].y].man = 0;
    }
    for (i = 0; i < P; i++) {
        elf[i].ok = 1;
    }
    // Se verifica daca a ratat ghetarul
    for (i = 0; i < P; i++) {
        if (dropzone(elf[i].x, elf[i].y, R, R) == 0) {
            fprintf(out, "%s has missed the glacier.\n", elf[i].nume);
            elf[i].ok = 0;
            ice[elf[i].x][elf[i].y].busy = 0;
        }
    }
}
// Functia ce muta elful cu o casuta in sus
int up(FILE *out, int id, struct elfs *elf, struct ices **ice,
    int *R, int R_orig, int P) {
    int d = 0, aux = 0;
    int r = 0;
    r = (int) *R;
    if (elf[id].x == R_orig - r && elf[id].y == r) {
        fprintf(out, "%s fell off the glacier.\n", elf[id].nume);
        elf[id].ok = 0;
        ice[elf[id].x][elf[id].y].busy = 0;
        return 0;
    } else {
        d = abs(ice[elf[id].x - 1][elf[id].y].alt -
            ice[elf[id].x][elf[id].y].alt);
        if (elf[id].stamina >= d) {
            elf[id].stamina -= d;
            ice[elf[id].x][elf[id].y].busy = 0;
            elf[id].x -= 1;
            if (ice[elf[id].x][elf[id].y].man > elf[id].dmg) {
                aux = elf[id].dmg;
                elf[id].dmg = ice[elf[id].x][elf[id].y].man;
                ice[elf[id].x][elf[id].y].man = aux;
            }
            if (ice[elf[id].x][elf[id].y].busy == 1) {
                fight(out, elf, ice, id, elf[id].x, elf[id].y, P);
                if (elf[id].hp <= 0) {
                    return 0;
                }
            } else {
                ice[elf[id].x][elf[id].y].busy = 1;
            }
            if (dropzone(elf[id].x, elf[id].y, r, R_orig) == 0) {
                    fprintf(out, "%s fell off the glacier.\n", elf[id].nume);
                    elf[id].ok = 0;
                    ice[elf[id].x][elf[id].y].busy = 0;
                    return 0;
            }
        }
    }
    return 1;
}
// Functia ce muta elful cu o casuta in jos
int down(FILE *out, int id, struct elfs *elf, struct ices **ice,
    int *R, int R_orig, int P) {
    int d = 0, aux = 0;
    int r = 0;
    r = (int) *R;
    if (elf[id].x == 2 * r && elf[id].y == r) {
        fprintf(out, "%s fell off the glacier.\n", elf[id].nume);
        elf[id].ok = 0;
        ice[elf[id].x][elf[id].y].busy = 0;
        return 0;
    } else {
        d = abs(ice[elf[id].x + 1][elf[id].y].alt -
            ice[elf[id].x][elf[id].y].alt);
        if (elf[id].stamina >= d) {
            elf[id].stamina -= d;
            ice[elf[id].x][elf[id].y].busy = 0;
            elf[id].x += 1;
            if (ice[elf[id].x][elf[id].y].man > elf[id].dmg) {
                aux = elf[id].dmg;
                elf[id].dmg = ice[elf[id].x][elf[id].y].man;
                ice[elf[id].x][elf[id].y].man = aux;
            }
            if (ice[elf[id].x][elf[id].y].busy == 1) {
                fight(out, elf, ice, id, elf[id].x, elf[id].y, P);
                if (elf[id].hp <= 0) {
                    return 0;
                }
            } else {
                ice[elf[id].x][elf[id].y].busy = 1;
            }
            if (dropzone(elf[id].x, elf[id].y, r, R_orig) == 0) {
                    fprintf(out, "%s fell off the glacier.\n", elf[id].nume);
                    elf[id].ok = 0;
                    ice[elf[id].x][elf[id].y].busy = 0;
                    return 0;
            }
        }
    }
    return 1;
}
// Functia ce muta elful cu o casuta in stanga
int left(FILE *out, int id, struct elfs *elf, struct ices **ice,
    int *R, int R_orig, int P) {
    int d = 0, aux = 0;
    int r = 0;
    r = (int) *R;
    if (elf[id].y == R_orig - r && elf[id].x == r) {
        fprintf(out, "%s fell off the glacier.\n", elf[id].nume);
        elf[id].ok = 0;
        ice[elf[id].x][elf[id].y].busy = 0;
        return 0;
    } else {
        d = abs(ice[elf[id].x][elf[id].y - 1].alt -
            ice[elf[id].x][elf[id].y].alt);
        if (elf[id].stamina >= d) {
            elf[id].stamina -= d;
            ice[elf[id].x][elf[id].y].busy = 0;
            elf[id].y -= 1;
            if (ice[elf[id].x][elf[id].y].man > elf[id].dmg) {
                aux = elf[id].dmg;
                elf[id].dmg = ice[elf[id].x][elf[id].y].man;
                ice[elf[id].x][elf[id].y].man = aux;
            }
            if (ice[elf[id].x][elf[id].y].busy == 1) {
                fight(out, elf, ice, id, elf[id].x, elf[id].y, P);
                if (elf[id].hp <= 0) {
                    return 0;
                }
            } else {
                ice[elf[id].x][elf[id].y].busy = 1;
            }
            if (dropzone(elf[id].x, elf[id].y, r, R_orig) == 0) {
                    fprintf(out, "%s fell off the glacier.\n", elf[id].nume);
                    elf[id].ok = 0;
                    ice[elf[id].x][elf[id].y].busy = 0;
                    return 0;
            }
        }
    }
    return 1;
}
// Functia ce muta elful cu o casuta in dreapta
int right(FILE *out, int id, struct elfs *elf, struct ices **ice,
    int *R, int R_orig, int P) {
    int d = 0, aux = 0;
    int r = 0;
    r = (int) *R;
    if (elf[id].y == 2 * r && elf[id].x == r) {
        fprintf(out, "%s fell off the glacier.\n", elf[id].nume);
        elf[id].ok = 0;
        ice[elf[id].x][elf[id].y].busy = 0;
        return 0;
    } else {
        d = abs(ice[elf[id].x][elf[id].y + 1].alt -
            ice[elf[id].x][elf[id].y].alt);
        if (elf[id].stamina >= d) {
            elf[id].stamina -= d;
            ice[elf[id].x][elf[id].y].busy = 0;
            elf[id].y += 1;
            if (ice[elf[id].x][elf[id].y].man > elf[id].dmg) {
                aux = elf[id].dmg;
                elf[id].dmg = ice[elf[id].x][elf[id].y].man;
                ice[elf[id].x][elf[id].y].man = aux;
            }
            if (ice[elf[id].x][elf[id].y].busy == 1) {
                fight(out, elf, ice, id, elf[id].x, elf[id].y, P);
                if (elf[id].hp <= 0) {
                    return 0;
                }
            } else {
                ice[elf[id].x][elf[id].y].busy = 1;
            }
            if (dropzone(elf[id].x, elf[id].y, r, R_orig) == 0) {
                    fprintf(out, "%s fell off the glacier.\n", elf[id].nume);
                    elf[id].ok = 0;
                    ice[elf[id].x][elf[id].y].busy = 0;
                    return 0;
            }
        }
    }
    return 1;
}
// Citeste sirul de caractere din miscarea MOVE si apeleaza functia
// corespunzatoare
void moves(FILE *out, struct elfs *elf, int id, char dir[],
    int *R, struct ices **ice, int P, int R_orig) {
    unsigned int i = 0;
    for (i = 0; i < strlen(dir); i++) {
        if (dir[i] == 'U') {
            if (up(out, id, elf, ice, R, R_orig, P) == 0) {
                break;
            }
        }
        if (dir[i] == 'D') {
            if (down(out, id, elf, ice, R, R_orig, P) == 0) {
                break;
            }
        }
        if (dir[i] == 'L') {
            if (left(out, id, elf, ice, R, R_orig, P) == 0) {
                break;
            }
        }
        if (dir[i] == 'R') {
            if (right(out, id, elf, ice, R, R_orig, P) == 0) {
                break;
            }
        }
    }
}
// Functia snowstorm ce lucreaza pe biti pentru a obtine numerele
void snows(FILE *out, long n, struct elfs *elf, struct ices **ice, int P) {
    int x = 0, y = 0, R = 0, DMG = 0, i = 0;
    int n1 = n, n2 = n, n3 = n;
    float d = 0.0;
    // Se shifteaza stanga si dreapta pentru a obtine numerele
    n1 = n << 24;
    x = n1 >> 24;
    n2 = n << 16;
    y = n2 >> 24;
    n3 = n << 8;
    R = n3 >> 24;
    DMG = n >> 24;
    DMG = DMG & 255;
    for (i = 0; i < P; i++) {
        if (elf[i].ok == 1) {
            if (R == 0) {
                if (elf[i].x == x && elf[i].y == y) {
                    elf[i].hp -= DMG;
                    if (elf[i].hp <= 0) {
                        elf[i].ok = 0;
                        fprintf(out, "%s was hit by snowstorm.\n", elf[i].nume);
                        ice[elf[i].x][elf[i].y].busy = 0;
                    }
                }
            } else {
                d = sqrt((x - elf[i].x) * (x - elf[i].x) +
                (y - elf[i].y) * (y - elf[i].y));
                if (d <= (float) R) {
                    elf[i].hp -= DMG;
                    if (elf[i].hp <= 0) {
                        elf[i].ok = 0;
                        fprintf(out, "%s was hit by snowstorm.\n", elf[i].nume);
                        ice[elf[i].x][elf[i].y].busy = 0;
                    }
                }
            }
        }
    }
}
// Functia meltdown ce creste stamina si verifica daca elfii inca sunt
// pe ghetar
void melts(FILE *out, int x, int *R, int P, struct elfs *elf,
    struct ices **ice, int R_orig) {
    int i = 0, r = 0;
    *R = *R - 1;
    r = (int) *R;
    for (i = 0; i < P; i++) {
        if (elf[i].ok == 1) {
            elf[i].stamina += x;
        }
    }
    for (i = 0; i < P; i++) {
        if (elf[i].ok == 1) {
            if (dropzone(elf[i].x, elf[i].y, r, R_orig) == 0) {
                fprintf(out, "%s got wet because of global warming.\n",
                elf[i].nume);
                elf[i].ok = 0;
                ice[elf[i].x][elf[i].y].busy = 0;
            }
        }
    }
}
// Functia ce printeaza tabela de castig
void scores(FILE *out, int P, struct elfs *elf, struct elfs *elf_ord) {
    int i = 0, j = 0, aux1 = 0, aux2 = 0;
    char aux[20], aux3[20];
    for (i = 0; i < P; i++) {
        snprintf(elf_ord[i].nume, sizeof(elf_ord[i].nume), "%s", elf[i].nume);
        elf_ord[i].ok = elf[i].ok;
        elf_ord[i].kills = elf[i].kills;
    }
    for (i = 0; i < P-1; i++) {
        for (j = i + 1; j < P; j++) {
            if (elf_ord[i].ok > elf_ord[j].ok) {
                    snprintf(aux, sizeof(aux), "%s", elf_ord[i].nume);
                    snprintf(elf_ord[i].nume, sizeof(elf_ord[i].nume), "%s",
                    elf_ord[j].nume);
                    snprintf(elf_ord[j].nume, sizeof(elf_ord[j].nume), "%s",
                    aux);
                    aux1 = elf_ord[i].ok;
                    elf_ord[i].ok = elf_ord[j].ok;
                    elf_ord[j].ok = aux1;
                    aux2 = elf_ord[i].kills;
                    elf_ord[i].kills = elf_ord[j].kills;
                    elf_ord[j].kills = aux2;
            }
        }
    }
    for (i = 0; i < P-1; i++) {
        for (j = i + 1; j < P; j++) {
            if (elf_ord[i].ok == elf_ord[j].ok) {
                if (elf_ord[i].kills < elf_ord[j].kills) {
                    snprintf(aux3, sizeof(aux3), "%s", elf_ord[i].nume);
                    snprintf(elf_ord[i].nume, sizeof(elf_ord[i].nume), "%s",
                    elf_ord[j].nume);
                    snprintf(elf_ord[j].nume, sizeof(elf_ord[j].nume), "%s",
                    aux3);
                    aux1 = elf_ord[i].ok;
                    elf_ord[i].ok = elf_ord[j].ok;
                    elf_ord[j].ok = aux1;
                    aux2 = elf_ord[i].kills;
                    elf_ord[i].kills = elf_ord[j].kills;
                    elf_ord[j].kills = aux2;
                }
            }
        }
    }
    for (i = 0; i < P-1; i++) {
        for (j = i + 1; j < P; j++) {
            if (elf_ord[i].ok == elf_ord[j].ok) {
                if (elf_ord[i].kills == elf_ord[j].kills) {
                    if (strcmp(elf_ord[i].nume, elf_ord[j].nume) > 0) {
                        snprintf(aux, sizeof(aux), "%s", elf_ord[i].nume);
                        snprintf(elf_ord[i].nume, sizeof(elf_ord[i].nume), "%s",
                        elf_ord[j].nume);
                        snprintf(elf_ord[j].nume, sizeof(elf_ord[j].nume), "%s",
                        aux);
                        aux1 = elf_ord[i].ok;
                        elf_ord[i].ok = elf_ord[j].ok;
                        elf_ord[j].ok = aux1;
                        aux2 = elf_ord[i].kills;
                        elf_ord[i].kills = elf_ord[j].kills;
                        elf_ord[j].kills = aux2;
                    }
                }
            }
        }
    }
    fprintf(out, "SCOREBOARD:\n");
    for (i = 0; i < P; i++) {
        if (elf_ord[i].ok == 1) {
            fprintf(out, "%s\tDRY\t%d\n", elf_ord[i].nume, elf_ord[i].kills);
        }
    }
    for (i = 0; i < P; i++) {
        if (elf_ord[i].ok == 0) {
            fprintf(out, "%s\tWET\t%d\n", elf_ord[i].nume, elf_ord[i].kills);
        }
    }
}
// O functie ce verifica daca jocul s-a incheiat
int end(FILE *out, struct elfs *elf, int P) {
    int i = 0, nr = 0, winner = 0;
    for (i = 0; i < P; i++) {
        if (elf[i].ok == 1) {
            nr++;
            winner = i;
        }
    }
    if (nr == 1) {
        fprintf(out, "%s has won.\n", elf[winner].nume);
        return 1;
    } else {
        return 0;
    }
}
// Functia ce citeste miscarile si apeleaza mai departe functiile
// corespunzatoare
void miscari(FILE *in, FILE *out, int *R, int P, struct elfs *elf,
    struct ices **ice, int R_orig, struct elfs *elf_ord) {
    char move[20] = "MOVE";
    char snow[20] = "SNOWSTORM";
    char melt[20] = "MELTDOWN";
    char score[20] = "PRINT_SCOREBOARD";
    int arg1 = 0, arg3 = 0;
    long arg2 = 0;
    while (1) {
        char cmd[20], dir[20];
        fscanf(in, "%s", cmd);
        if (strcmp(move, cmd) == 0) {
            fscanf(in, "%d", &arg1);
            fscanf(in, "%s", dir);
            if (elf[arg1].ok == 1) {
                moves(out, elf, arg1, dir, R, ice, P, R_orig);
            }
            if (end(out, elf, P)) {
                break;
            }
        }
        if (strcmp(snow, cmd) == 0) {
            fscanf(in, "%ld", &arg2);
            snows(out, arg2, elf, ice, P);
            if (end(out, elf, P)) {
                break;
            }
        }
        if (strcmp(melt, cmd) == 0) {
            fscanf(in, "%d", &arg3);
            melts(out, arg3, R, P, elf, ice, R_orig);
            if (end(out, elf, P)) {
                break;
            }
        }
        if (strcmp(score, cmd) == 0) {
            scores(out, P, elf, elf_ord);
            if (end(out, elf, P)) {
                break;
            }
        }
        if (feof(in)) {
            break;
        }
    }
}
// Se aloca si se elibereaza dinamic memoria si se deschid si
// inchid fisierele
int main(int argc, char * argv[]) {
    (void) argc;
    int R = 0, P = 0, i = 0;
    struct elfs *elf;
    struct elfs *elf_ord;
    struct ices **ice;
    char file_in[50];
    char file_out[50];
    snprintf(file_in, sizeof(file_in), "%s.in", argv[0]);
    snprintf(file_out, sizeof(file_out), "%s.out", argv[0]);
    FILE *in = fopen(file_in, "r");
    FILE *out = fopen(file_out, "w");
    fscanf(in, "%d%d", &R, &P);
    int R_orig = R;
    elf = (struct elfs*) malloc (P * sizeof(struct elfs));
    elf_ord = (struct elfs*) malloc (P * sizeof(struct elfs));
    ice = (struct ices **) malloc ((2 * R_orig + 1) * sizeof(struct ices *));
    for (i = 0; i < 2 * R_orig + 1; i++) {
        ice[i] = (struct ices *) malloc((2 * R_orig + 1)
            * sizeof(struct ices));
    }
    citire(ice, elf, R, P, in, out);
    if (end(out, elf, P) == 0) {
        miscari(in, out, &R, P, elf, ice, R_orig, elf_ord);
    }
    free(elf);
    free(elf_ord);
    for (i = 0; i < 2 * R_orig + 1; i++) {
        free(ice[i]);
    }
    free(ice);
    fclose(in);
    fclose(out);
    return 0;
}
