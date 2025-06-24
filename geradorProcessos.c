#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PROCESSOS 5

int main() {
    FILE *f = fopen("processos.txt", "w");
    
    if (!f) {
        printf("Erro ao criar processos.txt\n");
        return 1;
    }

    char IOarray[] = {'A','B','C','D'};
    srand(time(NULL));

    for (int i = 0; i < MAX_PROCESSOS; i++){
        int PID = i;
        int prioridade = 1;
        int feedback = 0;
        int status = 0;
        int tempoServico = 5 + (rand() % 15);
        int tempoEspera = 0;
        char necessidadeIO = IOarray[rand() % 4];
        int tempoIO = 0;

        switch (necessidadeIO) {
            case 'A': 
                tempoIO = 3; 
                break;
            case 'B': 
                tempoIO = 5; 
                break;
            case 'C': 
                tempoIO = 6; 
                break;
            case 'D': 
                tempoIO = 0; 
                break;
        }

        int tempoChegada = (i == 0) ? 0 : rand() % 20;
        int tempoInicioIO = (necessidadeIO != 'D') ? tempoChegada + 2 + (rand() % 4) : 0;

        // Formato: PID tempoChegada prioridade status tempoServico feedback necessidadeIO tempoIO tempoInicioIO tempoEspera
        fprintf(f, "%d %d %d %d %d %d %c %d %d %d\n",
            PID, tempoChegada, prioridade, status, tempoServico, feedback,
            necessidadeIO, tempoIO, tempoInicioIO, tempoEspera);
    }

    fclose(f);
    printf("Processos gerados com sucesso!\n");

    return 0;
}